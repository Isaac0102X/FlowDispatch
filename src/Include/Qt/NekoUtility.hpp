#ifndef NEKO_UTILITY_HPP
#define NEKO_UTILITY_HPP

#include "3rdparty/base64.h"

#include <random>

#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QMessageBox>
#include <QRegularExpression>
#include <QTcpServer>
#include <QTimer>
#include <QUrlQuery>
#include <QFileInfo>
#include <QTextDocument>
#include <QTextCursor>

// #ifdef Q_OS_WIN
// #include "sys/windows/guihelper.h"
// #endif

#include <cassert>
#include <utility>

#ifdef QT_DEBUG
#define GFW_ASSERT(expr) assert(expr)
#else
#define GFW_ASSERT(expr) ((void)0)
#endif

#define FIRST_OR_SECOND(a, b) a.isEmpty() ? b : a

inline const QString UNICODE_LRO = QString::fromUtf8(QByteArray::fromHex("E280AD"));

#define Int2String(num) QString::number(num)

inline QString SubStrBefore(QString str, const QString &sub) {
    if (!str.contains(sub)) return str;
    return str.left(str.indexOf(sub));
    }

inline QString SubStrAfter(QString str, const QString &sub) {
    if (!str.contains(sub)) return str;
    return str.right(str.length() - str.indexOf(sub) - sub.length());
    }

QString QStringList2Command(const QStringList &list);

QStringList SplitLinesSkipSharp(const QString &_string, int maxLine = 0);

// Base64

QByteArray DecodeB64IfValid(const QString &input, QByteArray::Base64Options options = QByteArray::Base64Option::Base64Encoding);

// URL

class QUrlQuery;

#define GetQuery(url) QUrlQuery((url).query(QUrl::ComponentFormattingOption::FullyDecoded));

QString GetQueryValue(const QUrlQuery &q, const QString &key, const QString &def = "");

QString GetRandomString(int randomStringLength);

quint64 GetRandomUint64();

// JSON

class QJsonObject;
class QJsonArray;

QJsonObject QString2QJsonObject(const QString &jsonString);

QString QJsonObject2QString(const QJsonObject &jsonObject, bool compact);

template<typename T>
QJsonArray QList2QJsonArray(const QList<T> &list);

QList<int> QJsonArray2QListInt(const QJsonArray &arr);

#define QJSONARRAY_ADD(arr, add) \
for (const auto &a: (add)) { \
        (arr) += a;              \
    }
#define QJSONOBJECT_COPY(src, dst, key) \
if (src.contains(key)) dst[key] = src[key];
#define QJSONOBJECT_COPY2(src, dst, src_key, dst_key) \
if (src.contains(src_key)) dst[dst_key] = src[src_key];

QList<QString> QJsonArray2QListString(const QJsonArray &arr);

namespace
    {
    inline QByteArray ReadFile(const QString &path)
        {
        QFile file(path);
        GFW_ASSERT(true==file.open(QFile::ReadOnly));
        return file.readAll();
        }

    inline QString ReadFileText(const QString &path)
        {
        QFile file(path);
        GFW_ASSERT(true==file.open(QFile::ReadOnly | QFile::Text));
        QTextStream stream(&file);
        return stream.readAll();
        }

    // Validators

    inline bool IsIpAddress(const QString &str)
        {
        auto address = QHostAddress(str);
        if (address.protocol() == QAbstractSocket::IPv4Protocol ||
            address.protocol() == QAbstractSocket::IPv6Protocol)
            return true;
        return false;
        }

    inline bool IsIpAddressV4(const QString &str)
        {
        auto address = QHostAddress(str);
        if (address.protocol() == QAbstractSocket::IPv4Protocol)
            return true;
        return false;
        }

    inline bool IsIpAddressV6(const QString &str)
        {
        auto address = QHostAddress(str);
        if (address.protocol() == QAbstractSocket::IPv6Protocol)
            return true;
        return false;
        }

    // [2001:4860:4860::8888] -> 2001:4860:4860::8888
    inline QString UnwrapIPV6Host(QString &str) {
        return str.replace("[", "").replace("]", "");
        }

    // [2001:4860:4860::8888] or 2001:4860:4860::8888 -> [2001:4860:4860::8888]
    inline QString WrapIPV6Host(QString &str) {
        if (!IsIpAddressV6(str)) return str;
        return "[" + UnwrapIPV6Host(str) + "]";
        }

    inline QString DisplayAddress(QString serverAddress, int serverPort) {
        if (serverAddress.isEmpty() && serverPort == 0) return {};
        return WrapIPV6Host(serverAddress) + ":" + Int2String(serverPort);
        };

    inline bool InRange(unsigned x, unsigned low, unsigned high) {
        return (low <= x && x <= high);
        }

    inline bool IsValidPort(int port) {
        return InRange(port, 1, 65535);
        }

    // UI
    inline QWidget *GetMessageBoxParent(QWidget *mainwindow = nullptr)
        {
        auto activeWindow = QApplication::activeWindow();
        if (activeWindow == nullptr && mainwindow != nullptr)
            {
            if (mainwindow->isVisible())
                return mainwindow;
            return nullptr;
            }
        return activeWindow;
        }

    template<typename EMITTER, typename SIGNAL, typename RECEIVER, typename ReceiverFunc>
    inline void connectOnce(EMITTER *emitter, SIGNAL signal, RECEIVER *receiver, ReceiverFunc f,
        Qt::ConnectionType connectionType = Qt::AutoConnection) {
        auto connection = std::make_shared<QMetaObject::Connection>();
        auto onTriggered = [connection, f](auto... arguments) {
            std::invoke(f, arguments...);
            QObject::disconnect(*connection);
        };

        *connection = QObject::connect(emitter, signal, receiver, onTriggered, connectionType);
        }

    inline QString FindCoreAsset(const QString &name)
        {
        QStringList search{};
        search << QApplication::applicationDirPath();
        search << "/usr/share/sing-geoip";
        search << "/usr/share/sing-geosite";
        search << "/usr/share/sing-box";
        search << "/usr/lib/nekobox";
        search << "/usr/share/nekobox";
        for (const auto &dir : std::as_const(search))
            {
            if (dir.isEmpty())
                continue;
            QFileInfo asset(dir + "/" + name);
            if (asset.exists())
                {
                return asset.absoluteFilePath();
                }
            }
        return {};
        }

    //获取 nekobox_core 的真实路径
    inline QString FindNekoCore()
        {
        auto fn = QApplication::applicationDirPath() + "/nekobox_core";
        auto fi = QFileInfo(fn);
        //如果是符号链接则返回实际位置
        if (fi.isSymLink())
            return fi.symLinkTarget();
        return fn;
        }

    // IsAdmin 主要判断：有无权限启动 Tun
    // bool IsAdmin()
    //     {
    //     short isAdminCache = -1;
    //     if (isAdminCache >= 0)
    //         return isAdminCache;

    //     bool admin = false;
    // #ifdef Q_OS_WIN
    //     admin = Windows_IsInAdmin();
    // #else
    // #ifdef Q_OS_LINUX
    //     admin |=
    //         Linux_GetCapString(FindNekoBoxCoreRealPath()).contains("cap_net_admin");
    // #endif
    //     admin |= geteuid() == 0;
    // #endif

    //     isAdminCache = admin;
    //     return admin;
    //     };


    QStringList SplitLines(const QString &_string)
        {
    #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        return _string.split(QRegularExpression("[\r\n]"),
                             Qt::SplitBehaviorFlags::SkipEmptyParts);
    #else
        return _string.split(QRegularExpression("[\r\n]"), QString::SkipEmptyParts);
    #endif
        }

    inline QStringList SplitLinesSkipSharp(const QString &_string, int maxLine)
        {
        auto lines = SplitLines(_string);
        QStringList newLines;
        int i = 0;
        for (const auto &line : std::as_const(lines))
            {
            if (line.trimmed().startsWith("#"))
                continue;
            newLines << line;
            if (maxLine > 0 && ++i >= maxLine)
                break;
            }
        return newLines;
        }

    inline QString QStringList2Command(const QStringList &list)
        {
        QStringList new_list;
        for (auto str : list)
            {
            auto q = "\"" + str.replace("\"", "\\\"") + "\"";
            new_list << q;
            }
        return new_list.join(" ");
        }

    inline QString GetQueryValue(const QUrlQuery &q, const QString &key,
                          const QString &def)
        {
        auto a = q.queryItemValue(key);
        if (a.isEmpty())
            {
            return def;
            }
        return a;
        }

    inline QString GetRandomString(int randomStringLength)
        {
        std::random_device rd;
        std::mt19937 mt(rd());

        const QString possibleCharacters(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

        std::uniform_int_distribution<int> dist(0, possibleCharacters.length() - 1);

        QString randomString;
        for (int i = 0; i < randomStringLength; ++i)
            {
            QChar nextChar = possibleCharacters.at(dist(mt));
            randomString.append(nextChar);
            }
        return randomString;
        }

    inline quint64 GetRandomUint64()
        {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<quint64> dist;
        return dist(mt);
        }

    inline int MkPort()
        {
        QTcpServer s;
        s.listen();
        auto port = s.serverPort();
        s.close();
        return port;
        }

    inline QString ReadableSize(const qint64 &size)
        {
        double sizeAsDouble = size;
        static QStringList measures;
        if (measures.isEmpty())
            measures << "B"
                     << "KiB"
                     << "MiB"
                     << "GiB"
                     << "TiB"
                     << "PiB"
                     << "EiB"
                     << "ZiB"
                     << "YiB";
        QStringListIterator it(measures);
        QString measure(it.next());
        while (sizeAsDouble >= 1024.0 && it.hasNext())
            {
            measure = it.next();
            sizeAsDouble /= 1024.0;
            }
        return QString::fromLatin1("%1 %2")
            .arg(sizeAsDouble, 0, 'f', 2)
            .arg(measure);
        }



    inline QString DisplayTime(long long time, int formatType)
        {
        QDateTime t;
        t.setMSecsSinceEpoch(time * 1000);
        return QLocale().toString(t, QLocale::FormatType(formatType));
        }

    inline void ActivateWindow(QWidget *w)
        {
        w->setWindowState(w->windowState() & ~Qt::WindowMinimized);
        w->setVisible(true);
    #ifdef Q_OS_WIN
        Windows_QWidget_SetForegroundWindow(w);
    #endif
        w->raise();
        w->activateWindow();
        }


    inline void setTimeout(const std::function<void()> &callback, QObject *obj,
                    int timeout)
        {
        auto t = new QTimer;
        QObject::connect(t, &QTimer::timeout, obj,
                         [=]
                         {
                             callback();
                             t->deleteLater();
                         });
        t->setSingleShot(true);
        t->setInterval(timeout);
        t->start();
        }

    inline void FastAppendTextDocument(const QString &message, QTextDocument *doc)
        {
        QTextCursor cursor(doc);
        cursor.movePosition(QTextCursor::End);
        cursor.beginEditBlock();
        cursor.insertBlock();
        cursor.insertText(message);
        cursor.endEditBlock();
        }
    }
#endif