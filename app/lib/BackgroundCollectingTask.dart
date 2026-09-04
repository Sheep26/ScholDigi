import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';
import 'package:scoped_model/scoped_model.dart';
import 'package:hive/hive.dart';

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

  int expectedSize = 44;
  int expectedPointSize = 24;

  Exercise? currExercise;
  STATUS status = STATUS.idle;

  BackgroundCollectingTask._fromConnection(this._connection) {
    _connection.input!.listen((data) {
      _buffer += data;

      while (true) {
        // If there is a sample, and it is full sent
        int index = _buffer.indexOf('t'.codeUnitAt(0));

        if (index >= 0 && _buffer.length - index >= expectedSize && status == STATUS.idle) {
          final Exercise sample = Exercise(distance: _buffer[index], avgSpeed: _buffer[index + 4], topSpeed: _buffer[index + 8], avgAlt: _buffer[index + 12], minAlt: _buffer[index + 16], maxAlt: _buffer[index + 20], altDiff: _buffer[index + 24], start: _buffer[index + 28], stop: _buffer[index + 36]);
          _buffer.removeRange(0, index + expectedSize);
          currExercise = sample;

          status = STATUS.collecting;
          notifyListeners();
        } else if (index >= 0 && _buffer.length - index >= expectedPointSize && status == STATUS.collecting) {
          final ExercisePoint sample = ExercisePoint(lat: _buffer[index], lng: _buffer[index + 4], alt: _buffer[index + 8], time: _buffer[index + 12], speed: _buffer[index + 20]);

          currExercise?.points.add(sample);
          _buffer.removeRange(0, index + expectedPointSize);
        } else {
          status = STATUS.idle;
          box.put(box.keys.length.toString(), currExercise);
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
