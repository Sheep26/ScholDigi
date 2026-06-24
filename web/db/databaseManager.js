import { initDB, db } from './connection.js';

export class DatabaseManager {
    constructor(db_name) {
        this.DBNAME = db_name;

        (async () => {
            await initDB(this.DBNAME);

            await db.execute(`CREATE TABLE IF NOT EXISTS users (user_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY, firstname TEXT NOT NULL, lastname TEXT NOT NULL, email TEXT NOT NULL, passwd_hash TEXT NOT NULL)`);
            await db.execute(`CREATE TABLE IF NOT EXISTS trips (trip_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY, user_id INT UNSIGNED NOT NULL, location_points JSON NOT NULL DEFAULT (JSON_ARRAY()), speed_points JSON NOT NULL DEFAULT (JSON_ARRAY()), avg_speed FLOAT NOT NULL)`);
        })();
    }
}