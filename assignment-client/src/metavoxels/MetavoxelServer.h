//
//  MetavoxelServer.h
//  hifi
//
//  Created by Andrzej Kapolka on 12/18/13.
//  Copyright (c) 2013 High Fidelity, Inc. All rights reserved.
//

#ifndef __hifi__MetavoxelServer__
#define __hifi__MetavoxelServer__

#include <QList>
#include <QTimer>

#include <ThreadedAssignment.h>

#include <DatagramSequencer.h>
#include <MetavoxelData.h>

class MetavoxelEditMessage;
class MetavoxelSession;

/// Maintains a shared metavoxel system, accepting change requests and broadcasting updates.
class MetavoxelServer : public ThreadedAssignment {
    Q_OBJECT

public:
    
    MetavoxelServer(const QByteArray& packet);

    void applyEdit(const MetavoxelEditMessage& edit);

    const MetavoxelData& getData() const { return _data; }

    virtual void run();
    
    virtual void readPendingDatagrams();
    
private slots:

    void maybeAttachSession(const SharedNodePointer& node);
    void sendDeltas();    
    
private:
    
    QTimer _sendTimer;
    qint64 _lastSend;
    
    MetavoxelData _data;
};

/// Contains the state of a single client session.
class MetavoxelSession : public NodeData {
    Q_OBJECT
    
public:
    
    MetavoxelSession(MetavoxelServer* server, const SharedNodePointer& node);
    virtual ~MetavoxelSession();

    virtual int parseData(const QByteArray& packet);

    void sendDelta();

private slots:

    void sendData(const QByteArray& data);

    void readPacket(Bitstream& in);    
    
    void clearSendRecordsBefore(int index);
    
    void handleMessage(const QVariant& message);
    
private:
    
    class SendRecord {
    public:
        int packetNumber;
        MetavoxelData data;
        MetavoxelLOD lod;
    };
    
    MetavoxelServer* _server;
    
    DatagramSequencer _sequencer;
    
    SharedNodePointer _node;
    
    MetavoxelLOD _lod;
    
    QList<SendRecord> _sendRecords;
};

#endif /* defined(__hifi__MetavoxelServer__) */
