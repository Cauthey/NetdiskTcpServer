#include "mytcpsocket.h"
#include <QDebug>
#include "protocol.h"

MyTcpSocket::MyTcpSocket()
{
    connect(this,SIGNAL(readyRead()),this,SLOT(recvMsg()));
    connect(this,SIGNAL(disconnected()),this,SLOT(clientOffine()));
}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::recvMsg()
{
    qDebug() << this->bytesAvailable();
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen,sizeof(uint));
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));
//    qDebug() << pdu->uiMsgType << (char*)(pdu->caMsg);
    switch(pdu->uiMsgType){
    case ENUM_MSG_TYPE_REGIST_REQUEST:
    {
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName,pdu->caData,32);
        strncpy(caPwd,pdu->caData+32,32);
        bool ret = OpeDB::getInstance().handleRegist(caName,caPwd);
        PDU *respdu  =  mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
        if (ret){
            strcpy(respdu->caData,REGIST_SUCCESS);
        }else{
            strcpy(respdu->caData,REGIST_FAILED);
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_REQUEST:
    {
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName,pdu->caData,32);
        strncpy(caPwd,pdu->caData+32,32);
        bool ret = OpeDB::getInstance().handleLogin(caName,caPwd);
        PDU *respdu  =  mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
        if (ret){
            strcpy(respdu->caData,LOGIN_SUCCESS);
            m_strName = caName;
        }else{
            strcpy(respdu->caData,LOGIN_FAILED);
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;

    }
    case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
    {
        QStringList res = OpeDB::getInstance().handleAllOnline();
        uint uiMsgLen = res.size()*32;
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType=ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
        for(int i=0;i<res.size();i++){
        memcpy((char*)respdu->caMsg+i*32
                ,res.at(i).toStdString().c_str()
                 ,res.at(i).size());
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
    }
    case ENUM_MSG_TYPE_SEARCH_USER_REQUEST:
    {
        int res = OpeDB::getInstance().handleSearchUser(pdu->caData);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_RESPOND;
        if(-1==res){
            strcpy(respdu->caData,SEARCH_USR_NO);
        }else if(1==res){
            strcpy(respdu->caData,SEARCH_USR_ONLINE);
        }else{
            strcpy(respdu->caData,SEARCH_USR_OFFLINE);
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    default:
        break;
    }
    free(pdu);
    pdu = NULL;


//    qDebug() << caName << caPwd << pdu->uiMsgType << endl;


}

void MyTcpSocket::clientOffine()
{
    OpeDB::getInstance().handleOffine(m_strName.toStdString().c_str());
    emit offine(this);





}
