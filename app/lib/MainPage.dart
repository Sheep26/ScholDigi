import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

import './BackgroundCollectingTask.dart';
import './DiscoveryPage.dart';
import './TrampPage.dart';

import 'package:hive/hive.dart';

// import './helpers/LineChart.dart';

class MainPage extends StatefulWidget {
  const MainPage({super.key});

  @override
  _MainPage createState() => _MainPage();
}

class _MainPage extends State<MainPage> {
  BluetoothAdapterState _bluetoothState = BluetoothAdapterState.unknown;

  BackgroundCollectingTask? _collectingTask;

  int _selectedIndex = 0;
  BluetoothDevice? connectedDevice;

  StreamSubscription<BluetoothAdapterState>? _adapterSubscription;
  StreamSubscription<BluetoothConnectionState>? _connectionSubscription;

  final box = Hive.box();

  @override
  void initState() {
    super.initState();

    _adapterSubscription =
        FlutterBluePlus.adapterState.listen((state) {
      if (!mounted) return;

      setState(() {
        _bluetoothState = state;
      });
    });
  }

  @override
  void dispose() {
    _adapterSubscription?.cancel();
    _connectionSubscription?.cancel();

    // Don't disconnect here: the connection may be used elsewhere.
    super.dispose();
  }

  void _onItemTapped(int index) {
    setState(() {
      _selectedIndex = index;
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Uhhhh ermmm'),
      ),
      body: <Widget>[ListView(
        children: <Widget>[
          SwitchListTile(
            title: const Text('Enable Bluetooth'),
            value: _bluetoothState == BluetoothAdapterState.on,
            onChanged: (bool value) {
              
            },
          ),
          Divider(),
          ListTile(
            title: ElevatedButton(
                child: const Text('Explore discovered devices'),
                onPressed: () async {
                  final device = await Navigator.of(context).push(
                    MaterialPageRoute(
                      builder: (context) {
                        return DiscoveryPage();
                      },
                    ),
                  );

                  connectedDevice = device;
                  _startBackgroundTask(context, device);
                }),
          )
        ],
      ),
      ListView.builder(
        padding: const EdgeInsets.all(8.0),
        itemCount: box.keys.length + 1,
        itemBuilder: (context, index) {
          // First item: Start/Stop button
          if (index == 0) {
            return Padding(
              padding: const EdgeInsets.all(8.0),
              child: ElevatedButton(
                child: Text(_collectingTask?.inProgress == true ? 'Stop Tramp' : 'Start Tramp'),
                onPressed: () async {
                  final task = _collectingTask;

                  if (task == null) {
                    ScaffoldMessenger.of(context).showSnackBar(const SnackBar(content: Text('Connect to a device first.'),),);
                    return;
                  }

                  if (task.inProgress) {
                    await task.stop();
                  } else {
                    await task.start();
                  }

                  if (mounted) {
                    setState(() {});
                  }
                },
              ),
            );
          }

          // Remaining items: saved tramps
          final keys = box.keys.toList();
          final key = keys[keys.length - index];
          final tramp = box.get(key);

          final start = DateTime.fromMillisecondsSinceEpoch((tramp['start'] as num).toInt());

          return Card(
            child: ListTile(
              leading: const Icon(Icons.nordic_walking),
              title: Text('Tramp ${box.keys.length - index + 1}'),
              subtitle: Text('${start.hour}:${start.minute} ${start.day}/${start.month}/${start.year}'),
              onTap: () {
                Navigator.of(context).push(
                  MaterialPageRoute(
                    builder: (context) => TrampPage(
                      tramp: tramp,
                      title: 'Tramp ${box.keys.length - index + 1}',
                    ),
                  ),
                );
              },
            ),
          );
        },
      ),
      ][_selectedIndex],
      bottomNavigationBar: BottomNavigationBar(
        items: const <BottomNavigationBarItem>[
          BottomNavigationBarItem(icon: Icon(Icons.bluetooth), label: 'Bluetooth'),
          BottomNavigationBarItem(icon: Icon(Icons.nordic_walking), label: 'Tramps'),
        ],
        currentIndex: _selectedIndex,
        selectedItemColor: Colors.amber[800],
        onTap: _onItemTapped,
      ),
    );
  }

  /* void _startChat(BuildContext context, BluetoothDevice server) {
    Navigator.of(context).push(
      MaterialPageRoute(
        builder: (context) {
          return ChatPage(server: server);
        },
      ),
    );
  } */

  Future<void> _startBackgroundTask(BuildContext context, BluetoothDevice server) async {
    try {
      // The device must already be connected by DiscoveryPage.
      _collectingTask = await BackgroundCollectingTask.connect(server);

      // await _collectingTask!.start();

      if (!context.mounted) return;

      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text('Exercise collection started'),
        ),
      );
    } catch (ex) {
      await _collectingTask?.dispose();
      _collectingTask = null;

      if (!context.mounted) return;

      showDialog<void>(
        context: context,
        builder: (dialogContext) => AlertDialog(
          title: const Text('Error starting collection'),
          content: Text(ex.toString()),
          actions: [
            TextButton(
              onPressed: () => Navigator.of(dialogContext).pop(),
              child: const Text('Close'),
            ),
          ],
        ),
      );
    }
  }
}
