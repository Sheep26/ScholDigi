import 'package:flutter/material.dart';
import 'package:hive/hive.dart';
import 'package:path_provider/path_provider.dart';

import './MainPage.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  final dir = await getApplicationDocumentsDirectory();
  Hive.defaultDirectory = dir.path;

  Hive.box().clear();
  Hive.box().put('0', {"distance": 0, "avgSpeed": 0, "topSpeed": 0, "avgAlt": 0, "minAlt": 0, "maxAlt": 0, "altDiff": 0, "start": 0, "stop": 0});

  runApp(App());
}

class App extends StatelessWidget {
  const App({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(home: MainPage());
  }
}
