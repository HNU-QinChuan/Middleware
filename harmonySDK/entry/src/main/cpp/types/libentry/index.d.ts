export const runCommand: (input: string, recordPath: string, index: number) => string;
export const stopCommand: (command: string) => string;
export const registerMessageCallback: ( cb: (message: string) => void) => void;



