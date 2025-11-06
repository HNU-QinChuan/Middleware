import fs, { ReadOptions, ListFileOptions } from '@ohos.file.fs';
import buffer from '@ohos.buffer';



/**
 * 创建文件存到沙箱中用于测试
 * @param filesDir  沙箱路径
 * 创建了十个文件
 * @returns
 */
export function  createFile(filesDir: string): string {
  if (!filesDir) {
    console.error("filesDir is not initialized.");
    return ("错误：文件目录为空。");
  }
  try {
    //新建并打开文件
    for(let i = 0; i < 10; i++)
    {
      let filePath = filesDir + '/text' + i + '.txt';
      let file = fs.openSync(filePath ,fs.OpenMode.READ_WRITE|fs.OpenMode.CREATE);
      //写
      let writeLen = fs.writeSync(file.fd,"try to write str" + i);
      console.info("The length of str is" + writeLen);
      //读
      let arrayBuffer = new ArrayBuffer(1024);
      let readOptions:ReadOptions = {
        offset: 0,
        length: arrayBuffer.byteLength
      }
      let readLen = fs.readSync(file.fd, arrayBuffer, readOptions);
      let buf = buffer.from(arrayBuffer, 0, readLen);
      console.info("the content of file: " + buf.toString());
      // 关闭文件
      fs.closeSync(file);
    }

    return ("初始化完成，已创建10个文件并刷新列表。");
  } catch (e) {
    console.error("Error in createFile: " + JSON.stringify(e));
    return ("创建文件时出错: " + e.message);
  }
}




/**
 * 获取沙箱中文件列表
 * @param filesDir  沙箱路径
 * @returns   沙箱中的所有文件（string[]类型返回）
 */
export function getFileList(filesDir: string): string[] {
  let listFileOption: ListFileOptions = {
    recursion: false,
    listNum: 0,
    filter: {
      suffix: [".png", ".jpg", ".txt"],
      displayName: ["*"],
      fileSizeOver: 0,
      lastModifiedAfter: new Date(0).getTime()
    }
  };
  // 使用 this.filesDir
  let files = fs.listFileSync(filesDir, listFileOption);
  let fileNames: string[] = [];

  for (let i = 0; i < files.length; i++) {
    console.info(`The name of file: ${files[i]}`);
    fileNames.push(files[i]);
  }
  return fileNames;
}

/**
 * 读取列表中的文件
 * @param filesDir  文件的路径（沙箱路径）
 * @param fileName  文件名称
 * @returns   返回值为文件内容（字符串类型）
 */
export function readFile(filesDir: string, fileName: string): string {
  let filePath = filesDir + '/' + fileName;
  let file = fs.openSync(filePath ,fs.OpenMode.READ_WRITE|fs.OpenMode.CREATE);
  //读
  let arrayBuffer = new ArrayBuffer(1024);
  let readOptions:ReadOptions = {
    offset: 0,
    length: arrayBuffer.byteLength
  }
  let readLen = fs.readSync(file.fd, arrayBuffer, readOptions);
  let buf = buffer.from(arrayBuffer, 0, readLen);
  // 关闭文件
  fs.closeSync(file);
  return buf.toString();

}

//TODO:写文件接口，传入参数：沙箱位置、文件名、写入内容。逻辑：检查沙箱内有无该文件，有直接打开继续写入，无则新建文件后写入。
export function writeIntoFile(filesDir: string, fileName: string, content: string):void {
  let filePath = filesDir + '/' + fileName + '.txt';
  let file = fs.openSync(filePath ,fs.OpenMode.APPEND|fs.OpenMode.READ_WRITE|fs.OpenMode.CREATE);
  //写
  let writeLen = fs.writeSync(file.fd, content + '\n');
  console.info("The length of str is" + writeLen);
  // 关闭文件
  fs.closeSync(file);

}
