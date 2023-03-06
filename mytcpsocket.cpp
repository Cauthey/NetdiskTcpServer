#include "mytcpsocket.h"
#include <QDebug>
#include<stdio.h>
#include"mytcpserver.h"
#include "protocol.h"
#include <QFileInfoList>


MyTcpSocket::MyTcpSocket()
{
    m_bUpload = false;

    connect(this,SIGNAL(readyRead()),this,SLOT(recvMsg()));
    connect(this,SIGNAL(disconnected()),this,SLOT(clientOffine()));
}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::recvMsg()
{
    if(!m_bUpload){
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
                // 创建用户文件夹
                QDir dir;
                qDebug() << dir.mkdir(QString("./%1").arg(caName));
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
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName,pdu->caData,32);
            strncpy(caName,pdu->caData+32,32);
            int res = OpeDB::getInstance().handAddFriend(caPerName,caName);
            PDU *respdu = NULL;
            if(-1==res){
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
                strcpy(respdu->caData,UNKNOWN_ERROR);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }else if(0==res){
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
                strcpy(respdu->caData,FRIEND_EXISTS);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }else if(1==res){
                MyTcpServer::getInstance().resend(caPerName,pdu);


            }else if(2==res){
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
                strcpy(respdu->caData,USER_OFFINE);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }else if(3==res){
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
                strcpy(respdu->caData,USER_NOT_EXISTS);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }else{
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
                strcpy(respdu->caData,UNKNOWN_ERROR);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName,pdu->caData,32);
            strncpy(caName,pdu->caData+32,32);
            OpeDB::getInstance().handleAgreeAddFriend(caPerName,caName);
            MyTcpServer::getInstance().resend(caName,pdu);
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
        {
            char caName[32]= {'\0'};
            strncpy(caName,pdu->caData+32,32);
            MyTcpServer::getInstance().resend(caName,pdu);
            break;
        }
        case ENUM_MSG_TYPE_FRIEND_FLUSH_REQUEST:
        {
            char caName[32] = {'\0'};
            strncpy(caName,pdu->caData,32);
            QStringList res = OpeDB::getInstance().handFlushFriend(caName);
            uint uiMsgLen = res.size() * 32 ;
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_FRIEND_FLUSH_RESPOND;
            for(int i=0;i<res.size();i++){
                memcpy((char*)(respdu->caMsg)+i*32,res.at(i).toStdString().c_str(),res.at(i).size());
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char selfName[32] = {'\0'};
            char friendName[32] = {'\0'};
            strncpy(selfName,pdu->caData,32);
            strncpy(friendName,pdu->caData+32,32);
            OpeDB::getInstance().handledelFriend(selfName,friendName);

            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
            strcpy(respdu->caData,DEL_FRIEND_OK);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            MyTcpServer::getInstance().resend(friendName,pdu);

            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char chatName[32] = {'\0'};
            strncpy(chatName,pdu->caData+32,32);
            qDebug() << chatName;
            MyTcpServer::getInstance().resend(chatName,pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            char caName[32] = {'\0'};
            strncpy(caName,pdu->caData,32);
            QStringList onlineFriend = OpeDB::getInstance().handFlushFriend(caName);
            QString tempName;
            for(int i=0;i<onlineFriend.size();i++){
                tempName = onlineFriend.at(i);
                MyTcpServer::getInstance().resend(tempName.toStdString().c_str(),pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
        {
            QDir dir;
            QString strCurPath = QString("%1").arg((char*)pdu->caMsg);
            bool res = dir.exists(strCurPath);
            PDU *respdu = NULL;
            if(res){  //当前目录已存在
                char caNewDir[32] = {'\0'};
                memcpy(caNewDir,pdu->caData+32,32);
                QString strNewPath = strCurPath + "/" +caNewDir;
                qDebug() << "---->" <<strNewPath;
                if(dir.exists(strNewPath)){  // 创建的文件名已存在
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData,FILE_NAME_EXISTS);
                } else{
                    dir.mkdir(strNewPath);
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData,CREATE_DIR_SUCCESS);
                }
            }
            else    // 当前目录不存在
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                strcpy(respdu->caData,DIR_NO_EXISTS);
            }
            write((char*)respdu,respdu->uiPDULen);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
        {
            char *pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath,pdu->caMsg,pdu->uiMsgLen);
            QDir dir(pCurPath);
            QFileInfoList fileList =  dir.entryInfoList();
            int iFileCount= fileList.size();
            PDU *respdu = mkPDU(sizeof(FileInfo)*iFileCount);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
            FileInfo *pFileInfo = NULL;
            QString strFileName;
            for(int i=0;i<iFileCount;i++)
            {
                pFileInfo = (FileInfo*)(respdu->caMsg)+i;
                strFileName = fileList[i].fileName();
                memcpy(pFileInfo->caName,strFileName.toStdString().c_str(),strFileName.size());
                if(fileList[i].isDir()){
                    pFileInfo->iFileType = 0;
                }
                else if(fileList[i].isFile()){
                    pFileInfo->iFileType = 1;
                }
                qDebug() << fileList[i].fileName()
                         << fileList[i].size()
                         << "文件：" << fileList[i].isFile()
                         << "文件夹：" << fileList[i].isDir();
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_REQUEST:
        {
            char caName[32]={'\0'};
            strcpy(caName,pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caName);
            qDebug()<< strPath;
            QFileInfo fileInfo(strPath);
            bool res = false;
            if(fileInfo.isDir()){
                QDir dir;
                dir.setPath(strPath);
                res = dir.removeRecursively();
            }else if(fileInfo.isFile()){
                res=false;
            }
            PDU *respdu=NULL;
            if(res){
                respdu = mkPDU(strlen(DEL_DIR_SUCCESS)+1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(respdu->caData,DEL_DIR_SUCCESS,strlen(DEL_DIR_SUCCESS));
            }else{
                respdu = mkPDU(strlen(DEL_DIR_FAILED)+1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(respdu->caData,DEL_DIR_FAILED,strlen(DEL_DIR_FAILED));
            }

            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_RENAME_DIR_REQUEST:
        {
            char caOldName[32]={'\0'};
            char caNewName[32]={'\0'};
            strncpy(caOldName,pdu->caData,32);
            strncpy(caNewName,pdu->caData+32,32);

            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);

            QString strOldPath = QString("%1/%2").arg(pPath).arg(caOldName);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(caNewName);

            QDir dir;
            bool ret = dir.rename(strOldPath,strNewPath);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_DIR_RESPOND;
            if(ret){
                strcpy(respdu->caData,RENAME_SUCCESS);
            }else{
                strcpy(respdu->caData,RENAME_FAILED);
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
        {
            char caEnterName[32]={'\0'};
            strncpy(caEnterName,pdu->caData,32);

            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);

            QString strPath = QString("%1/%2").arg(pPath).arg(caEnterName);

            qDebug() << strPath;

            QFileInfo fileInfo(strPath);

            PDU *respdu = NULL;
            if(fileInfo.isDir()){
                QDir dir(strPath);
                QFileInfoList fileList =  dir.entryInfoList();
                int iFileCount = fileList.size();
                respdu = mkPDU(sizeof(FileInfo)*iFileCount);
                respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
                FileInfo *pFileInfo = NULL;
                QString strFileName;
                for(int i=0;i<iFileCount;i++)
                {
                    pFileInfo = (FileInfo*)(respdu->caMsg) + i;
                    strFileName = fileList[i].fileName();
                    qDebug() << strFileName;
                    strncpy(pFileInfo->caName,strFileName.toStdString().c_str(),strFileName.size());
                    if(fileList[i].isDir()){
                        pFileInfo->iFileType = 0;
                    }
                    else if(fileList[i].isFile()){
                        pFileInfo->iFileType = 1;
                    }
                }
            }else if(fileInfo.isFile()){
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                strcpy(respdu->caData,ENTER_DIR_FAILED);
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_REQUEST:
        {
            char caName[32]={'\0'};
            strcpy(caName,pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caName);
            qDebug()<< strPath;
            qDebug()<< strPath;
            qDebug()<< strPath;
            QFileInfo fileInfo(strPath);
            bool res = false;
            if(fileInfo.isDir()){
               res=false;
            }else if(fileInfo.isFile()){
               QDir dir;
               res = dir.remove(strPath);
            }
            PDU *respdu=NULL;
            if(res){
                respdu = mkPDU(strlen(DEL_FILE_SUCCESS)+1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
                memcpy(respdu->caData,DEL_FILE_SUCCESS,strlen(DEL_FILE_SUCCESS));
            }else{
                respdu = mkPDU(strlen(DEL_FILE_FAILED)+1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
                memcpy(respdu->caData,DEL_FILE_FAILED,strlen(DEL_FILE_FAILED));
            }

            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            char caFileName[32] = {'\0'};
            qint64 fileSize =0 ;
            sscanf(pdu->caData,"%s %lld",caFileName,&fileSize);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
            qDebug()<< strPath;
            delete []pPath;
            pPath = NULL;

            m_file.setFileName(strPath);
            // 以只写的方式打开文件,若文件不存在,则会自动创建文件
            if(m_file.open(QIODevice::WriteOnly)){
                // 此处可优化的多线程
                m_bUpload=true;
                m_iTotal = fileSize;
                m_iRecived = 0;
            }
            break;
        }
        default:
            break;
        }
        free(pdu);
        pdu = NULL;
    }else{
        // 接收数据



    }
}

void MyTcpSocket::clientOffine()
{
    OpeDB::getInstance().handleOffine(m_strName.toStdString().c_str());
    emit offine(this);
}
