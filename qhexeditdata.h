#ifndef QHEXEDITDATA_H
#define QHEXEDITDATA_H

/*
 * Piece Chain Data Structure Documentation From:
 * 1) http://www.catch22.net/tuts/piece-chains
 * 2) http://www.catch22.net/tuts/memory-techniques-part-1
 * 3) http://www.catch22.net/tuts/memory-techniques-part-2
 */

#include <QtCore>
#include <QUndoStack>
#include <QUndoCommand>

class QHexEditData : public QObject
{
    Q_OBJECT

    private:
        class ModifiedItem
        {
            public:
                ModifiedItem(qint64 pos, qint64 len, bool mod = true): _pos(pos), _len(len), _mod(mod) {}
                qint64 pos() const { return this->_pos; }
                qint64 length() const { return this->_len; }
                bool modified() const { return this->_mod; }
                void updatePos(qint64 amt) { this->_pos += amt; }
                void updateLen(qint64 amt) { this->_len += amt; }

            private:
                qint64 _pos;
                qint64 _len;
                bool _mod;
        };

        class ByteRef
        {
            public:
                ByteRef(qint64 pos, QHexEditData* o): _owner(o), _pos(pos) { }
                ByteRef(const ByteRef& br): _owner(br._owner), _pos(br._pos) { }
                ByteRef& operator =(uchar b) { this->_owner->replace(this->_pos, b); return *this; }
                operator uchar() { return this->_owner->at(this->_pos); }

            private:
                QHexEditData* _owner;
                qint64 _pos;
        };

        typedef QList<ModifiedItem*> ModifyList;

    public:
        class AbstractCommand: public QUndoCommand
        {
            public:
                AbstractCommand(qint64 pos, QHexEditData* owner, QUndoCommand* parent = 0): QUndoCommand(parent), _owner(owner), _notify(true), _pos(pos) { }

            public:
                bool canNotify() const { return this->_notify; }
                void setNotify(bool b) { this->_notify = b; }
                qint64 pos() const { return this->_pos; }
                QHexEditData* owner() const { return this->_owner; }

            private:
                QHexEditData* _owner;
                bool _notify;
                qint64 _pos;
        };

        class ModifyRangeCommand: public AbstractCommand
        {
            public:
                ModifyRangeCommand(int index, qint64 pos, const ModifyList& oldml, const ModifyList& newml, QHexEditData* owner, QUndoCommand* parent = 0): AbstractCommand(pos, owner, parent), _index(index), _oldml(oldml), _newml(newml)
                {
                    this->_oldlength = this->_newlength = 0;

                    for(int i = 0; i < oldml.length(); i++)
                        this->_oldlength += oldml[i]->length();

                    for(int i = 0; i < newml.length(); i++)
                        this->_newlength += newml[i]->length();
                }

            protected:
                qint64 index() const { return this->_index; }
                qint64 oldLength() const { return this->_oldlength; }
                qint64 newLength() const { return this->_newlength; }

            private:
                int _index;
                qint64 _oldlength;
                qint64 _newlength;

            protected:
                QHexEditData::ModifyList _oldml;
                QHexEditData::ModifyList _newml;
        };

        class InsertCommand: public ModifyRangeCommand
        {
            public:
                InsertCommand(int index, qint64 pos, const ModifyList& oldml, const ModifyList& newml, QHexEditData* owner, bool opt, QUndoCommand* parent = 0): ModifyRangeCommand(index, pos, oldml, newml, owner, parent), _optimized(opt) { }
                virtual int id() const { return QHexEditData::Insert; }

                virtual bool mergeWith(const QUndoCommand* command)
                {
                    const InsertCommand* ic = dynamic_cast<const InsertCommand*>(command);

                    if(ic->optimized())
                    {
                        ModifiedItem* oldmi = nullptr;

                        if(this->_newml.length() == 1) /* Single Item */
                            oldmi = this->_newml[0];
                        else /* Splitted Span in Three Parts, the second is the new one, update length */
                            oldmi = this->_newml[1];

                        ModifiedItem* newmi = ic->_newml.last();

                        oldmi->updateLen(newmi->length());
                        return true;
                    }

                    return false;
                }

                virtual void undo()
                {
                    for(int i = 0; i < this->_newml.length(); i++)
                        this->owner()->_modlist.removeAt(this->index());

                    for(int i = 0; i < this->_oldml.length(); i++)
                        this->owner()->_modlist.insert(this->index() + i, this->_oldml[i]);

                    if(this->canNotify())
                        emit this->owner()->dataChanged(this->pos(), this->oldLength(), QHexEditData::Insert);
                }

                virtual void redo()
                {
                    if(this->_optimized)
                        return; /* Don't modify the list if this is an optimized insertion */

                    for(int i = 0; i < this->_oldml.length(); i++)
                        this->owner()->_modlist.removeAt(this->index());

                    for(int i = 0; i < this->_newml.length(); i++)
                        this->owner()->_modlist.insert(this->index() + i, this->_newml[i]);

                    if(this->canNotify())
                        emit this->owner()->dataChanged(this->pos(), this->newLength(), QHexEditData::Insert);
                }

            private:
                bool optimized() const { return this->_optimized; }

            private:
                bool _optimized;
        };

        class RemoveCommand: public ModifyRangeCommand
        {
            public:
                RemoveCommand(int index, qint64 pos, const ModifyList& oldml, const ModifyList& newml, QHexEditData* owner, QUndoCommand* parent = 0): ModifyRangeCommand(index, pos, oldml, newml, owner, parent) { }
                virtual int id() const { return QHexEditData::Remove; }

                virtual void undo()
                {
                    this->owner()->_modlist.erase(this->owner()->_modlist.begin() + this->index(), this->owner()->_modlist.begin() + (this->index() + this->_newml.length()));

                    for(int i = 0; i < this->_oldml.length(); i++)
                        this->owner()->_modlist.insert(this->index() + i, this->_oldml.at(i));

                    if(this->canNotify())
                        emit this->owner()->dataChanged(this->pos(), this->oldLength(), QHexEditData::Remove);
                }

                virtual void redo()
                {
                    this->owner()->_modlist.erase(this->owner()->_modlist.begin() + this->index(), this->owner()->_modlist.begin() + (this->index() + this->_oldml.length()));

                    for(int i = 0; i < this->_newml.length(); i++)
                        this->owner()->_modlist.insert(this->index() + i, this->_newml.at(i));

                    if(this->canNotify())
                        emit this->owner()->dataChanged(this->pos(), this->newLength(), QHexEditData::Remove);
                }
        };

        class ReplaceCommand: public AbstractCommand
        {
            public:
                ReplaceCommand(qint64 pos, qint64 len, const QByteArray& ba, QHexEditData* owner, QUndoCommand* parent = 0): AbstractCommand(pos, owner, parent), _len(len), _data(ba), _remcmd(nullptr), _inscmd(nullptr) { }
                virtual int id() const { return QHexEditData::Replace; }

                virtual void undo()
                {
                    this->_inscmd->undo();
                    this->_remcmd->undo();

                    if(this->canNotify())
                        emit this->owner()->dataChanged(this->pos(), this->_data.length(), QHexEditData::Replace);
                }

                virtual void redo()
                {
                    if(this->_remcmd && this->_inscmd)
                    {
                        this->_remcmd->redo();
                        this->_inscmd->redo();

                    }
                    else
                    {
                        qint64 l = qMin(this->_len, this->owner()->length());

                        this->_remcmd = this->owner()->internalRemove(this->pos(), l, QHexEditData::Replace);
                        this->_remcmd->setNotify(false); /* Do not emit signal */
                        this->_remcmd->redo();

                        this->_inscmd = this->owner()->internalInsert(this->pos(), this->_data, QHexEditData::Replace);
                        this->_inscmd->setNotify(false);  /* Do not emit signal */
                        this->_inscmd->redo();
                    }

                    if(this->canNotify())
                        emit this->owner()->dataChanged(this->pos(), this->_data.length(), QHexEditData::Replace);
                }

            private:
                qint64 _len;
                QByteArray _data;
                RemoveCommand* _remcmd;
                InsertCommand* _inscmd;
        };

    public:
        explicit QHexEditData(QIODevice* iodevice, QObject *parent = 0);

    public:
        enum ActionType { None = 0, Insert = 1, Remove = 2, Replace = 3 };
        QUndoStack* undoStack();
        uchar at(qint64 i);
        qint64 indexOf(const QByteArray& ba, qint64 start);
        void append(const QByteArray& ba);
        void insert(qint64 pos, uchar ch);
        void insert(qint64 pos, const QByteArray& ba);
        void remove(qint64 pos, qint64 len);
        void replace(qint64 pos, uchar b);
        void replace(qint64 pos, qint64 len, uchar b);
        void replace(qint64 pos, const QByteArray& ba);
        void replace(qint64 pos, qint64 len, const QByteArray& ba);
        QByteArray read(qint64 pos, qint64 len);
        qint64 length() const;

    public slots:
        bool save();
        bool saveTo(QIODevice* iodevice);

    public:
        static QHexEditData* fromDevice(QIODevice* iodevice);
        static QHexEditData* fromFile(QString filename);
        static QHexEditData* fromMemory(const QByteArray& ba);

    private:
        InsertCommand* internalInsert(qint64 pos, const QByteArray& ba, QHexEditData::ActionType act);
        RemoveCommand* internalRemove(qint64 pos, qint64 len, QHexEditData::ActionType act);
        QHexEditData::ModifiedItem *modifiedItem(qint64 pos, qint64 *datapos = nullptr, int* index = nullptr);
        qint64 updateBuffer(const QByteArray& ba);
        bool canOptimize(QHexEditData::ActionType at, qint64 pos);
        void recordAction(QHexEditData::ActionType at, qint64 pos);

    signals:
        void dataChanged(qint64 offset, qint64 size, QHexEditData::ActionType reason);

    private:        
        ModifyList _modlist;
        QIODevice* _iodevice;
        QByteArray _modbuffer;
        qint64 _length;
        qint64 _lastpos;
        QHexEditData::ActionType _lastaction;
        QUndoStack _undostack;

};

#endif // QHEXEDITDATA_H
