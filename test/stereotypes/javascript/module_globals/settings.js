// The globals of the module settings, which the module app reaches through the name of the module

export let level = 1;
export const names = [];

// @stereotype global-command
export function raiseLevel() {
    level += 1;
}
