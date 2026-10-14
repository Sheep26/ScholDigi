import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';

import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:hive/hive.dart';
import 'package:scoped_model/scoped_model.dart';
import 'package:flutter/material.dart';

const String uartServiceUuid = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
const String uartWriteUuid = '6e400002-b5a3-f393-e0a9-e50e24dcca9e';
const String uartNotifyUuid = '6e400003-b5a3-f393-e0a9-e50e24dcca9e';

class ExercisePoint {
  double lat;
  double lng;
  double alt;
  int time;
  double speed;

  ExercisePoint({
    required this.lat,
    required this.lng,
    required this.alt,
    required this.time,
    required this.speed,
  });

  Map<String, dynamic> toJson() => {
        'lat': lat,
        'lng': lng,
        'alt': alt,
        'time': time,
        'speed': speed,
      };
}

class Exercise {
  double distance;
  double avgSpeed;
  double topSpeed;
  double altDiff;
  double avgAlt;
  double minAlt;
  double maxAlt;
  int start;
  int stop;

  List<ExercisePoint> points = [];

  Exercise({
    required this.distance,
    required this.avgSpeed,
    required this.topSpeed,
    required this.altDiff,
    required this.avgAlt,
    required this.minAlt,
    required this.maxAlt,
    required this.start,
    required this.stop,
  });

  Map<String, dynamic> toJson() => {
        'distance': distance,
        'avgSpeed': avgSpeed,
        'topSpeed': topSpeed,
        'altDiff': altDiff,
        'avgAlt': avgAlt,
        'minAlt': minAlt,
        'maxAlt': maxAlt,
        'start': start,
        'stop': stop,
        'points': points.map((p) => p.toJson()).toList(),
      };
}

enum STATUS {
  idle,
  collecting,
}

class BackgroundCollectingTask extends Model {
  static BackgroundCollectingTask of(BuildContext context, { bool rebuildOnChange = false,}) => ScopedModel.of<BackgroundCollectingTask>(context, rebuildOnChange: rebuildOnChange,);

  final BluetoothDevice _device;
  final BluetoothCharacteristic _writeCharacteristic;
  final BluetoothCharacteristic _notifyCharacteristic;

  final List<int> _buffer = [];
  final Box box = Hive.box();

  StreamSubscription<List<int>>? _notificationSubscription;
  StreamSubscription<BluetoothConnectionState>? _connectionSubscription;

  bool inProgress = false;
  bool _disposed = false;
  bool _savedCurrentExercise = false;

  final int expectedSize = 72;
  final int expectedPointSize = 40;

  Exercise? currExercise;
  STATUS status = STATUS.idle;

  BackgroundCollectingTask._(this._device, this._writeCharacteristic, this._notifyCharacteristic,) {
    _connectionSubscription = _device.connectionState.listen((state) {
      if (state == BluetoothConnectionState.disconnected) {
        inProgress = false;
        notifyListeners();
      }
    });

    _notificationSubscription =
        _notifyCharacteristic.onValueReceived.listen(
      _onDataReceived,
      onError: (Object error) {
        debugPrint('BLE notification error: $error');
      },
    );
  }

  static Future<BackgroundCollectingTask> connect(BluetoothDevice device,) async {
    if (!device.isConnected) {
      throw Exception('BLE device is not connected');
    }

    final services = await device.discoverServices();

    BluetoothCharacteristic? writeCharacteristic;
    BluetoothCharacteristic? notifyCharacteristic;

    for (final service in services) {
      if (service.uuid.toString().toLowerCase() !=
          uartServiceUuid) {
        continue;
      }

      for (final characteristic in service.characteristics) {
        final uuid = characteristic.uuid.toString().toLowerCase();

        if (uuid == uartWriteUuid) {
          writeCharacteristic = characteristic;
        } else if (uuid == uartNotifyUuid) {
          notifyCharacteristic = characteristic;
        }
      }
    }

    if (writeCharacteristic == null) {
      throw Exception('BLE UART write characteristic not found');
    }

    if (notifyCharacteristic == null) {
      throw Exception('BLE UART notify characteristic not found');
    }

    await notifyCharacteristic.setNotifyValue(true);

    return BackgroundCollectingTask._(
      device,
      writeCharacteristic,
      notifyCharacteristic,
    );
  }

  double _readDouble(int index) {
    final bytes = Uint8List.fromList(_buffer);

    return ByteData.sublistView(bytes).getFloat64(index, Endian.little);
  }

  int _readInt64(int index) {
    final bytes = Uint8List.fromList(_buffer.sublist(index, index + 8));

    return ByteData.sublistView(bytes).getInt64(0, Endian.little);
  }

  void _onDataReceived(List<int> data) {
    if (_disposed || data.isEmpty) return;

    if (ascii.decode(data, allowInvalid: true).trim() == 'DONE') {
      debugPrint('DONE received');
      _handleDone();
      return;
    }

    _buffer.addAll(data);

    while (_buffer.isNotEmpty) {
      // Header is 72 bytes; each point is 40 bytes.
      final recordSize = status == STATUS.idle ? expectedSize : expectedPointSize;

      // Wait for the entire record if it spans multiple notifications.
      if (_buffer.length < recordSize) break;

      if (status == STATUS.idle) {
        // Parse exercise header.
        currExercise = Exercise(
          distance: _readDouble(0),
          avgSpeed: _readDouble(8),
          topSpeed: _readDouble(16),
          altDiff: _readDouble(24),
          avgAlt: _readDouble(32),
          minAlt: _readDouble(40),
          maxAlt: _readDouble(48),
          start: _readInt64(56),
          stop: _readInt64(64),
        );

        _savedCurrentExercise = false;
        status = STATUS.collecting;

        _buffer.removeRange(0, expectedSize);

        debugPrint('Received exercise header');
      } else {
        // Parse one exercise point.
        final point = ExercisePoint(
          lat: _readDouble(0),
          lng: _readDouble(8),
          alt: _readDouble(16),
          time: _readInt64(24),
          speed: _readDouble(32),
        );

        currExercise?.points.add(point);
        _buffer.removeRange(0, expectedPointSize);
      }
      // notifyListeners();
    }
  }

  Future<void> _sendCommand(String command) async {
    if (!_device.isConnected) {
      throw Exception('BLE device is disconnected');
    }

    await _writeCharacteristic.write(
      ascii.encode(command),
      withoutResponse: _writeCharacteristic.properties.writeWithoutResponse && !_writeCharacteristic.properties.write,
    );
  }

  Future<void> start() async {
    if (!_device.isConnected) {
      throw Exception('BLE device is disconnected');
    }

    _buffer.clear();
    currExercise = null;
    status = STATUS.idle;
    _savedCurrentExercise = false;
    inProgress = true;

    notifyListeners();

    await _sendCommand('toggle');
  }

  Future<void> stop() async {
    await _sendCommand('toggle');
  }

  void _handleDone() {
    _saveCurrentExercise();

    inProgress = false;
    status = STATUS.idle;
    _buffer.clear();

    notifyListeners();

    debugPrint('Exercise saved: ${currExercise?.points.length} points');

    inProgress = false;
    currExercise = null;
    status = STATUS.idle;
    _savedCurrentExercise = false;
    _buffer.clear();
  }

  void _saveCurrentExercise() {
    final exercise = currExercise;

    if (exercise == null || _savedCurrentExercise) return;

    box.put(box.keys.length.toString(), exercise.toJson());
    _savedCurrentExercise = true;
  }

  Future<void> dispose() async {
    _disposed = true;

    await _notificationSubscription?.cancel();
    await _connectionSubscription?.cancel();

    try {
      await _notifyCharacteristic.setNotifyValue(false);
    } catch (_) {
      // The device may already be disconnected.
    }

    // super.dispose();
  }
}