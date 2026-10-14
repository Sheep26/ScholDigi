import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

class DiscoveryPage extends StatefulWidget {
  const DiscoveryPage({super.key});

  @override
  State<DiscoveryPage> createState() => _DiscoveryPageState();
}

class _DiscoveryPageState extends State<DiscoveryPage> {
  StreamSubscription<List<ScanResult>>? _scanSubscription;
  final List<ScanResult> _results = [];

  bool _isScanning = false;
  bool _isConnecting = false;
  String? _error;

  @override
  void initState() {
    super.initState();
    startDiscovery();
  }

  Future<void> startDiscovery() async {
    try {
      await _scanSubscription?.cancel();

      if (!await FlutterBluePlus.isSupported) {
        throw Exception('Bluetooth LE is not supported');
      }

      final state = await FlutterBluePlus.adapterState.where((s) => s != BluetoothAdapterState.unknown).first;

      if (state != BluetoothAdapterState.on) {
        throw Exception('Please enable Bluetooth on your phone');
      }

      if (!mounted) return;

      setState(() {
        _results.clear();
        _error = null;
        _isScanning = true;
      });

      _scanSubscription = FlutterBluePlus.onScanResults.listen(
        (results) {
          if (!mounted) return;

          setState(() {
            for (final result in results) {
              final index = _results.indexWhere(
                (r) => r.device.remoteId ==
                    result.device.remoteId,
              );

              if (index >= 0) {
                _results[index] = result;
              } else {
                _results.add(result);
              }
            }
          });
        },
        onError: (Object e) {
          if (!mounted) return;
          setState(() {
            _error = 'BLE scan failed: $e';
            _isScanning = false;
          });
        },
      );

      await FlutterBluePlus.startScan(
        timeout: const Duration(seconds: 10),
      );

      if (mounted) {
        setState(() => _isScanning = false);
      }
    } catch (e) {
      if (mounted) {
        setState(() {
          _error = e.toString();
          _isScanning = false;
        });
      }
    }
  }

  @override
  void dispose() {
    _scanSubscription?.cancel();
    FlutterBluePlus.stopScan();
    super.dispose();
  }

  Future<void> _connect(BluetoothDevice device) async {
    if (_isConnecting) return;

    setState(() {
      _isConnecting = true;
      _error = null;
    });

    try {
      await FlutterBluePlus.stopScan();
      await _scanSubscription?.cancel();

      await device.connect(
        license: License.nonprofit,
        timeout: const Duration(seconds: 15),
      );

      if (!mounted) {
        await device.disconnect();
        return;
      }

      // Return the connected device to the calling page.
      Navigator.of(context).pop(device);
    } catch (e) {
      if (!mounted) return;

      setState(() {
        _error = 'Connection failed: $e';
        _isConnecting = false;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(_isScanning ? 'Scanning for BLE devices' : 'Discovered devices'),
        actions: [
          IconButton(
            onPressed: _isScanning ? null : startDiscovery,
            icon: const Icon(Icons.refresh),
          ),
        ],
      ),
      body: Column(
        children: [
          if (_error != null)
            Padding(
              padding: const EdgeInsets.all(12),
              child: Text(
                _error!,
                style: const TextStyle(color: Colors.red),
              ),
            ),
          Expanded(
            child: ListView.builder(
              itemCount: _results.length,
              itemBuilder: (context, index) {
                final result = _results[index];
                final device = result.device;

                final name = result.advertisementData.advName.isNotEmpty ? result.advertisementData.advName : device.platformName;

                return ListTile(
                  leading: const Icon(Icons.bluetooth),
                  title: Text(
                    name.isEmpty ? 'Unknown BLE device' : name,
                  ),
                  subtitle: Text(
                    '${device.remoteId} • '
                    '${result.rssi} dBm',
                  ),
                  trailing: const Icon(Icons.link),
                  onTap: _isConnecting ? null : () => _connect(device),
                );
              },
            ),
          ),
        ],
      ),
    );
  }
}
