import express from 'express';
import morgan from "morgan";
import { fileURLToPath } from 'node:url';
import { dirname } from 'node:path';
import { existsSync } from 'fs';
import path from 'path';
import { SessionManager, Session } from './sessions/sessionManager.js';
import cookieParser from 'cookie-parser';
import renderUtils from './renderUtils.js';
import config from "./config.json" with { type: "json" };
import { DatabaseManager } from './db/databaseManager.js';

// Get the path the website is being run out of.
const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const sessionManager = new SessionManager();
const db = new DatabaseManager(config.mysql_db_name);

const app = express();
const PORT = 8080;

// Setup middleware.
app.use(express.json());
app.use(express.urlencoded({ extended: true }));
app.set('view engine', 'ejs');
app.set('views', './templates');
app.use('/static', express.static('static'));
app.use(cookieParser())
app.use(morgan('combined'));

app.use(async (req, res, next) => {
    /*
    * This function catches all uncaught routes and sends either a 404 for if the content is missing or sends the requested webpage.
    * The purpose of this is to cut down on unnessacary routes.
    * All routes that haven't been caught yet will go here, as a result this will get the requested content from the uri and check if it exists.
    * If it exists, it will return it if not it goes to 404 not found.
    */

    const page = req.path.replace('/', '') || 'home';

    if (page != 'login' && (!req.cookies.session || !sessionManager.getSession(req.cookies.session)))
        return res.redirect("/login");

    // Check if requested content exists.
    if (!existsSync(`templates/${page}.ejs`))
        return res.status(404).render('base', { title: '404', content: '404', renderUtils: renderUtils });

    res.render('base', { title: page, content: page, renderUtils: renderUtils });
});

app.listen(PORT, '0.0.0.0', async function (err) {
    /*
    * This function is the event handler for the web server.
    * It is ran when the web server starts and when it errors.
    */

    // Catch errors.
    if (err) {
        console.log(err);

        process.exit(1);
    }

    console.log(`Server running on http://localhost:${PORT}`);
});