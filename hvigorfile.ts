import { appTasks } from '@ohos/hvigor-ohos-plugin';

export default {
    system: appTasks,  /* Built-in plugin of Hvigor. It cannot be modified. */
    plugins:[]         /* Custom plugin to extend the functionality of Hvigor. */
}

export const appCompileSdkVersion = '11'
export const appCompatibleSdkVersion = '11'

console.log("Using SDK Version:", appCompileSdkVersion)
console.log("OHOS SDK ENV:", process.env["OHOS_SDK_HOME"])


