import 'package:flutter/material.dart';

class TrampPage extends StatelessWidget {
	final dynamic tramp;
  final String title;

	const TrampPage({
		super.key,
		required this.tramp,
    required this.title,
	});

	@override
	Widget build(BuildContext context) {
    final start = DateTime.fromMillisecondsSinceEpoch((tramp['start'] as num).toInt());
    final stop = DateTime.fromMillisecondsSinceEpoch((tramp['stop'] as num).toInt());
    final distance = (tramp['distance'] as num).toDouble().toStringAsFixed(2);
    final avgSpeed = (tramp['avgSpeed'] as num).toDouble().toStringAsFixed(2);
    final topSpeed = (tramp['topSpeed'] as num).toDouble().toStringAsFixed(2);
    final altDiff = (tramp['altDiff'] as num).toDouble().toStringAsFixed(2);
    final avgAlt = (tramp['avgAlt'] as num).toDouble().toStringAsFixed(2);
    final minAlt = (tramp['minAlt'] as num).toDouble().toStringAsFixed(2);
    final maxAlt = (tramp['maxAlt'] as num).toDouble().toStringAsFixed(2);
		return Scaffold(
			appBar: AppBar(
				title: Text(title),
			),
			body: Padding(
				padding: const EdgeInsets.all(16.0),
				child: Column(
					crossAxisAlignment: CrossAxisAlignment.start,
					children: [
						Text(
							'Start: ${start.hour}:${start.minute} ${start.day}/${start.month}/${start.year}',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Stop: ${stop.hour}:${stop.minute} ${stop.day}/${stop.month}/${stop.year}',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Distance: ${distance}m',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Average Speed: ${avgSpeed}kmph',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Top Speed: ${topSpeed}kmph',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Altitude Difference: ${altDiff}m',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Average Altitude: ${avgAlt}m',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Min Altitude: ${minAlt}m',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Max Altitude: ${maxAlt}m',
							style: const TextStyle(fontSize: 18),
						),
					],
				),
			),
		);
	}
}