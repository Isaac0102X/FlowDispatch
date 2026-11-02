
##关于配置文件
先cd到build/${CMAKE_BINARY_DIR}下面去再运行
cmake --build . --target copy_config
cmake --install .
以上2种方式都可以(第2种在根目录的CMakeLists.txt里暂时注释掉了)，但是还有几个需要完善的地方
1. 用户装了v1.0，改了config.json
   你发 v1.1，用户解压覆盖（绿色版最常见的升级方式）→ 新包里的模板 config.json 把用户改过的那份冲掉了
2. "配置模板改了是下个版本的事"这句对新装用户成立，对升级用户不成立。v1.1 的程序新增了 advanced.renderMode 字段，但老用户手上的 config.json 是 v1.0 时代生成的，里面没这个字段——模板里写了也没用，因为程序读的是用户那份。
3. 写回要原子化：先写 config.json.tmp 再 rename，避免程序崩溃时留下半截 JSON 导致下次启动解析失败。
在v1阶段没必要搞那么复杂
小团队那层的真实代码，很多时候就这么多：
```
QSettings s(QSettings::IniFormat, QSettings::UserScope, "Org", "App");
QString theme = s.value("general/theme", "light").toString();  // 读不到用默认值
s.setValue("general/theme", "dark");                           // 写
```
三行，新装、升级、恢复出厂（删文件）全都自然成立。很多公司做到这里就停了，不是他们不懂，是投入产出比到此为止。
个人项目就更不用说了。
#有三件事无论如何留着
加起来不到 20 行，能挡掉绝大部分用户投诉：
1. 已存在就绝不覆盖——if (QFile::exists(path)) return; 一行。这是"升级不丢设置"的全部秘密。
2. 原子写——QSaveFile（写临时文件 + rename），或者自己写 .tmp 再替换。程序崩溃时不会留下半截 JSON。
3. 缺字段用默认值兜底、未知字段别丢——读的时候 value(key, 默认值)，写的时候基于原文件改而不是重建。这样加新字段不用写迁移代码，老用户自动兼容。
第 3 点尤其划算：它让"版本号 + 迁移"这件事在字段只是新增的场景下根本不需要。只有字段改名、类型变了、结构重排时才需要写迁移，那种情况一年碰不到两次。




