/****************************************************************************
** Meta object code from reading C++ file 'imchannelapprover.h'
**
** Created: Thu Jun 2 16:30:06 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "imchannelapprover.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'imchannelapprover.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IMChannelApprover[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: signature, parameters, type, tag, flags
      37,   19,   18,   18, 0x05,
     111,   93,   18,   18, 0x05,
     160,   93,   18,   18, 0x05,
     210,   93,   18,   18, 0x05,
     317,  283,   18,   18, 0x05,
     355,   18,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
     389,  369,   18,   18, 0x0a,
     423,  415,   18,   18, 0x0a,
     451,   18,   18,   18, 0x0a,
     473,  470,   18,   18, 0x08,
     515,  470,   18,   18, 0x08,
     557,  470,   18,   18, 0x08,
     607,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_IMChannelApprover[] = {
    "IMChannelApprover\0\0dispatchOperation\0"
    "addedDispatchOperation(Tp::ChannelDispatchOperationPtr)\0"
    "accountId,channel\0"
    "textChannelAvailable(QString,Tp::TextChannelPtr)\0"
    "callChannelAvailable(QString,Tpy::CallChannelPtr)\0"
    "fileTransferChannelAvailable(QString,Tp::IncomingFileTransferChannelPt"
    "r)\0"
    "accountId,contactId,operationPath\0"
    "incomingCall(QString,QString,QString)\0"
    "invalidated()\0operationObjectPath\0"
    "onCloseOperation(QString)\0running\0"
    "setApplicationRunning(bool)\0"
    "registerApprover()\0op\0"
    "onCallChannelReady(Tp::PendingOperation*)\0"
    "onTextChannelReady(Tp::PendingOperation*)\0"
    "onFileTransferChannelReady(Tp::PendingOperation*)\0"
    "onInvalidated()\0"
};

const QMetaObject IMChannelApprover::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_IMChannelApprover,
      qt_meta_data_IMChannelApprover, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IMChannelApprover::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IMChannelApprover::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IMChannelApprover::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IMChannelApprover))
        return static_cast<void*>(const_cast< IMChannelApprover*>(this));
    if (!strcmp(_clname, "Tp::AbstractClientApprover"))
        return static_cast< Tp::AbstractClientApprover*>(const_cast< IMChannelApprover*>(this));
    return QObject::qt_metacast(_clname);
}

int IMChannelApprover::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: addedDispatchOperation((*reinterpret_cast< const Tp::ChannelDispatchOperationPtr(*)>(_a[1]))); break;
        case 1: textChannelAvailable((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< Tp::TextChannelPtr(*)>(_a[2]))); break;
        case 2: callChannelAvailable((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< Tpy::CallChannelPtr(*)>(_a[2]))); break;
        case 3: fileTransferChannelAvailable((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< Tp::IncomingFileTransferChannelPtr(*)>(_a[2]))); break;
        case 4: incomingCall((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 5: invalidated(); break;
        case 6: onCloseOperation((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 7: setApplicationRunning((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: registerApprover(); break;
        case 9: onCallChannelReady((*reinterpret_cast< Tp::PendingOperation*(*)>(_a[1]))); break;
        case 10: onTextChannelReady((*reinterpret_cast< Tp::PendingOperation*(*)>(_a[1]))); break;
        case 11: onFileTransferChannelReady((*reinterpret_cast< Tp::PendingOperation*(*)>(_a[1]))); break;
        case 12: onInvalidated(); break;
        default: ;
        }
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void IMChannelApprover::addedDispatchOperation(const Tp::ChannelDispatchOperationPtr _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void IMChannelApprover::textChannelAvailable(const QString & _t1, Tp::TextChannelPtr _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void IMChannelApprover::callChannelAvailable(const QString & _t1, Tpy::CallChannelPtr _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void IMChannelApprover::fileTransferChannelAvailable(const QString & _t1, Tp::IncomingFileTransferChannelPtr _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void IMChannelApprover::incomingCall(const QString & _t1, const QString & _t2, const QString & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void IMChannelApprover::invalidated()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}
QT_END_MOC_NAMESPACE
