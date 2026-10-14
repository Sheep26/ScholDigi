import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';
import 'package:scoped_model/scoped_model.dart';
import 'package:hive/hive.dart';
import 'dart:typed_data';

class ExercisePoint {
  int lat;
  int lng;
  int alt;
  int time;
  int speed;

  ExercisePoint({
    required this.lat,
    required this.lng,
    required this.alt,
    required this.time,
    required this.speed
  });

  Map<String, dynamic> toJson() {
		return {
			'lat': lat,
			'lng': lng,
			'alt': alt,
			'time': time,
			'speed': speed,
		};
	}
}

class Exercise {
  int distance;
  int avgSpeed;
  int topSpeed;
  int altDiff;
  int avgAlt;
  int minAlt;
  int maxAlt;
  int start;
  int stop;

  List<ExercisePoint> points = List<ExercisePoint>.empty(growable: true);

  Exercise({
    required this.distance,
    required this.avgSpeed,
    required this.topSpeed,
    required this.altDiff,
    required this.avgAlt,
    required this.minAlt,
    required this.maxAlt,
    required this.start,
    required this.stop
  });

  Map<String, dynamic> toJson() {
		return {
			'distance': distance,
			'avgSpeed': avgSpeed,
			'topSpeed': topSpeed,
			'altDiff': altDiff,
			'avgAlt': avgAlt,
			'minAlt': minAlt,
			'maxAlt': maxAlt,
			'start': start,
			'stop': stop,
			'points': points.map((point) => point.toJson()).toList(),
		};
	}
}

enum STATUS {
  idle,
  collecting
}

class BackgroundCollectingTask extends Model {
  static BackgroundCollectingTask of(
    BuildContext context, {
    bool rebuildOnChange = false,
  }) =>
      ScopedModel.of<BackgroundCollectingTask>(
        context,
        rebuildOnChange: rebuildOnChange,
      );

  final BluetoothConnection _connection;
  List<int> _buffer = List<int>.empty(growable: true);

  bool inProgress = false;

  final box = Hive.box();

  int expectedSize = 72;
  int expectedPointSize = 44;

  Exercise? currExercise;
  STATUS status = STATUS.idle;

  int readDouble(int index) {
    final bytes = Uint8List.fromList(
      _buffer.sublist(index, index + 8),
    );

    return ByteData.sublistView(bytes).getFloat64(0, Endian.little).toInt();
  }

  int readInt64(int index) {
    final bytes = Uint8List.fromList(
      _buffer.sublist(index, index + 8),
    );

    return ByteData.sublistView(bytes).getInt64(0, Endian.little);
  }

  BackgroundCollectingTask._fromConnection(this._connection) {
    _connection.input!.listen((data) {
      _buffer += data;

      while (true) {
        // If there is a sample, and it is full sent
        int index = _buffer.indexOf('t'.codeUnitAt(0));

        if (index >= 0 && _buffer.length - index >= expectedSize && status == STATUS.idle) {
          final sample = Exercise(distance: readDouble(index), avgSpeed: readDouble(index + 8), topSpeed: readDouble(index + 16), altDiff: readDouble(index + 24), avgAlt: readDouble(index + 32), minAlt: readDouble(index + 40), maxAlt: readDouble(index + 48), start: readInt64(index + 56), stop: readInt64(index + 64),);
          _buffer.removeRange(0, index + expectedSize);
          currExercise = sample;

          status = STATUS.collecting;
          notifyListeners();
        } else if (index >= 0 && _buffer.length - index >= expectedPointSize && status == STATUS.collecting) {
          final ExercisePoint sample = ExercisePoint(lat: readDouble(index), lng: readDouble(index + 8), alt: readDouble(index + 16), time: readInt64(index + 24), speed: readDouble(index + 32));

          currExercise?.points.add(sample);
          _buffer.removeRange(0, index + expectedPointSize);
        } else {
          status = STATUS.idle;
          box.put(box.keys.length.toString(), currExercise?.toJson());
          break;
        }
      }
    }).onDone(() {
      inProgress = false;
      notifyListeners();
    });
  }

  static Future<BackgroundCollectingTask> connect(
      BluetoothDevice server) async {
    final BluetoothConnection connection =
        await BluetoothConnection.toAddress(server.address);
    return BackgroundCollectingTask._fromConnection(connection);
  }

  void dispose() {
    _connection.dispose();
  }

  Future<void> start() async {
    inProgress = true;
    _buffer.clear();
    currExercise = null;
    notifyListeners();
    _connection.output.add(ascii.encode('start'));
    await _connection.output.allSent;
  }

  Future<void> cancel() async {
    inProgress = false;
    notifyListeners();
    currExercise = null;
    _connection.output.add(ascii.encode('stop'));
    await _connection.finish();
  }

  Future<void> pause() async {
    inProgress = false;
    notifyListeners();
    _connection.output.add(ascii.encode('stop'));
    await _connection.output.allSent;
  }

  Future<void> reasume() async {
    inProgress = true;
    notifyListeners();
    _connection.output.add(ascii.encode('start'));
    await _connection.output.allSent;
  }
}
