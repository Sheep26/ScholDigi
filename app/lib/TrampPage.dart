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
							'Start: ${tramp['start']}',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Stop: ${tramp['stop']}',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Distance: ${tramp['distance']}',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Average Speed: ${tramp['avgSpeed']}',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Top Speed: ${tramp['topSpeed']}',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Altitude Difference: ${tramp['altDiff']}',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Average Altitude: ${tramp['avgAlt']}',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Min Altitude: ${tramp['minAlt']}',
							style: const TextStyle(fontSize: 18),
						),
						Text(
							'Max Altitude: ${tramp['maxAlt']}',
							style: const TextStyle(fontSize: 18),
						),
					],
				),
			),
		);
	}
}