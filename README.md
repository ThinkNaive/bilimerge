# bilimerge
bilibili blv file format merge
usage:
  1. Find folders from /sdcard/Android/data/tv.danmaku.bili/download/
  2. Copy bilibili android app download files (folder with numbers such as 6323484, which contains sub-folders with numbers such as (16,17,18)) to the executive file (compiled by ./make.sh)
  3. The files ready for use are as follows:
      <certain folder>
        <bilimerge/>
        <61367436>
          <16>
            <lua.flvxxx/>
            <danmaku.xml/>
            <entry.json/>
          </16>
          <17>
            <lua.flvxxx/>
            <danmaku.xml/>
            <entry.json/>
          </17>
        </61367436>
        <47328443>
          ...
        </47328443>
        ...
      </certain folder>
      p.s. support multiple folders(61367436,47328443,34234231)
    4. execute bilimerge : ./bilimerge
    5. it is expected to find all merged videos with video title in the <certain folder> if well executed.
  
  哔哩哔哩安卓缓存合并输出为视频
  使用方法：
    1、将/sdcard/Android/data/tv.danmaku.bili/download下所有文件夹复制到某个文件夹（<certain folder>）下。
    2、用./make.sh编译好程序bilimerge，并复制到<certain folder>。
    3、运行./bilimerge，如果正常，会在bash显示进度，并在文件夹下看到所有被合并的视频。
