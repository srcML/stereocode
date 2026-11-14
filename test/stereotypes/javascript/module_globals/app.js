// A global of another module is reached through the name of the module

import * as settings from "./settings.js";
import { level } from "./settings.js";
const config = require("./config.js");

// @stereotype unclassified
function readLevel() {
    return settings.level;
}

// A call on a global of another module other than a query method changes the global
// @stereotype global-command wrapper
function remember(name) {
    settings.names.push(name);
}

// A name without the module reaches only the globals of its own file, and an imported name is no variable
// @stereotype literal
function copiedLevel() {
    return level;
}

// A module that require loads from outside the archive holds no global of the archive
// @stereotype literal
function retries() {
    return config.retries;
}

class Monitor {
    constructor() {
        this.seen = 0;
    }

    // A global is no state of the object
    // @stereotype incidental
    bump() {
        settings.level += 1;
    }

    // @stereotype set
    watch() {
        this.seen = settings.level;
    }
}

// A name of a module outside the archive is no global
// @stereotype literal
function home() {
    return os.homedir;
}
