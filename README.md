# ʦʹϵͳTSMS

һ C ʵֵСͽʦʹϵͳ Windows ̨УͬʱҲ Linux/macOS  GCC ԡϵͳǿγҪеϵͳơ롢ļ洢ѯͳơ޸ĺͲԡ

## һ

### 1. ûɫ

- **ʦԱ**¼롢άʦϢ͹Ϣ
- **Ա**ѯʦʡ鿴ͳƽְƽʵ޸ĹĿ

### 2. 

1. **ʦϢ¼**¼ʦšԱ䡢šְơʡλͿۿ
2. **ļ־û洢**¼롢޸ġɾԶ浽 `data/teachers.csv`ʱԶȡʷݡ
3. **ʦʲѯ**
   - ʦŲѯ
   - ڲŲѯ
   - ʦѯչܣ
   - ѯȱ浽ͳһγҪ
4. **ʦͳ**ͳƽʦܶƽʡ߹ʡ͹ʣŻ͹ʡ
5. **ʦ޸**ʦ޸ְơʡ𡢿ۿڽְƺĹʵ
6. **ʦɾ**ʦɾְ¼¼չܣ
7. **ϷԼ**ŲΪҲظͽںΧıֶβܰӢĶţƻ CSV ʽ

### 3. ǹ

- **Windows **ṩ `build_windows.bat` Windows ʹ MinGW GCC  Visual Studio `cl` 롣Ŀıļʹ GBK 룬ƥ䳣 Windows ̨
- **ά**ÿϸעͣڿγ̴ͺչ
- **ֲ**Ŀ¼߼ Windows `_mkdir`  Linux/macOS `mkdir`

## ϵͳ

- ʹ `Teacher` ṹ屣һʦĻϢ͹Ŀ
- ʹ `TeacherNode` ϵͳеȫʦ¼
- ʹ `QueryNode` ÿβѯĽͳһӡ
- ʹ CSV ıļΪ־ûļڲ鿴ͱݡ
- Ӧʼ㹫ʽ

```text
Ӧ =  + λ +  - ۿ
```

## Windows 

###  1ʹűƼ

 Windows ʾ PowerShell ִУ

```bat
build_windows.bat
```

űȳ `gcc`δװ GCC Visual Studio  `cl`ɹУ

```bat
tsms.exe
```

###  2ֶʹ MinGW GCC

```bat
gcc -std=c11 -Wall -Wextra -O2 -finput-charset=GBK -fexec-charset=GBK -o tsms.exe src\main.c
./tsms.exe
```

###  3ֶʹ Visual Studio cl

ȴ Developer Command Prompt for VSִУ

```bat
cl /source-charset:gbk /execution-charset:gbk /W4 /O2 /Fe:tsms.exe src\main.c
.\tsms.exe
```

> ʱԶ Windows ̨/л GBKҳ 936ֿеԴļ/ű/ĵ GBK 棻ֶ룬뱣 GBK 

## ġLinux/macOS 

```bash
make
./tsms
```

Ҳֱʹã

```bash
make run
```

## 塢ݸʽ

 CSV ıʽ棬ÿһʦ

```text
,,Ա,,,ְ,,λ,,ۿ
```
