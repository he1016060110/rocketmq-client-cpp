/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "include/DefaultMQClient.h"
#include "Logging.h"
#include "MQClientFactory.h"
#include "MQClientManager.h"
#include "NameSpaceUtil.h"
#include "TopicPublishInfo.h"
#include "UtilAll.h"

namespace rocketmq {

#define ROCKETMQCPP_VERSION "1.2.5"
#define BUILD_DATE "02-12-2020"
// display version: strings bin/librocketmq.so |grep VERSION
const char* rocketmq_build_time = "VERSION: " ROCKETMQCPP_VERSION ", BUILD DATE: " BUILD_DATE " ";

//<!************************************************************************
DefaultMQClient::DefaultMQClient() {
  string NAMESRV_ADDR_ENV = "NAMESRV_ADDR";
  if (const char* addr = getenv(NAMESRV_ADDR_ENV.c_str()))
    m_namesrvAddr = addr;
  else
    m_namesrvAddr = "";

  m_instanceName = "DEFAULT";
  m_nameSpace = "";
  m_clientFactory = NULL;
  m_serviceState = CREATE_JUST;
  m_pullThreadNum = std::thread::hardware_concurrency();
  m_tcpConnectTimeout = 3000;        // 3s
  m_tcpTransportTryLockTimeout = 3;  // 3s
  m_unitName = "";
}

DefaultMQClient::~DefaultMQClient() {}

string DefaultMQClient::getMQClientId() const {
  string clientIP = UtilAll::getLocalAddress();
  string processId = UtilAll::to_string(getpid());
  // return processId + "-" + clientIP + "@" + m_instanceName;
  return clientIP + "@" + processId + "#" + m_instanceName;
}

//<!groupName;
const string& DefaultMQClient::getGroupName() const {
  return m_GroupName;
}

void DefaultMQClient::setGroupName(const string& groupname) {
  m_GroupName = groupname;
}

const string& DefaultMQClient::getNamesrvAddr() const {
  return m_namesrvAddr;
}

void DefaultMQClient::setNamesrvAddr(const string& namesrvAddr) {
  m_namesrvAddr = NameSpaceUtil::formatNameServerURL(namesrvAddr);
}

const string& DefaultMQClient::getNamesrvDomain() const {
  return m_namesrvDomain;
}

void DefaultMQClient::setNamesrvDomain(const string& namesrvDomain) {
  m_namesrvDomain = namesrvDomain;
}

const string& DefaultMQClient::getInstanceName() const {
  return m_instanceName;
}

void DefaultMQClient::setInstanceName(const string& instanceName) {
  m_instanceName = instanceName;
}
const string& DefaultMQClient::getNameSpace() const {
  return m_nameSpace;
}

void DefaultMQClient::setNameSpace(const string& nameSpace) {
  m_nameSpace = nameSpace;
}
void DefaultMQClient::createTopic(const string& key, const string& newTopic, int queueNum) {
  try {
    getFactory()->createTopic(key, newTopic, queueNum, m_SessionCredentials);
  } catch (MQException& e) {
    LOG_ERROR(e.what());
  }
}

int64 DefaultMQClient::earliestMsgStoreTime(const MQMessageQueue& mq) {
  return getFactory()->earliestMsgStoreTime(mq, m_SessionCredentials);
}

QueryResult DefaultMQClient::queryMessage(const string& topic, const string& key, int maxNum, int64 begin, int64 end) {
  return getFactory()->queryMessage(topic, key, maxNum, begin, end, m_SessionCredentials);
}

int64 DefaultMQClient::minOffset(const MQMessageQueue& mq) {
  return getFactory()->minOffset(mq, m_SessionCredentials);
}

int64 DefaultMQClient::maxOffset(const MQMessageQueue& mq) {
  return getFactory()->maxOffset(mq, m_SessionCredentials);
}

int64 DefaultMQClient::searchOffset(const MQMessageQueue& mq, uint64_t timestamp) {
  return getFactory()->searchOffset(mq, timestamp, m_SessionCredentials);
}

MQMessageExt* DefaultMQClient::viewMessage(const string& msgId) {
  return getFactory()->viewMessage(msgId, m_SessionCredentials);
}

vector<MQMessageQueue> DefaultMQClient::getTopicMessageQueueInfo(const string& topic) {
  boost::weak_ptr<TopicPublishInfo> weak_topicPublishInfo(
      getFactory()->tryToFindTopicPublishInfo(topic, m_SessionCredentials));
  boost::shared_ptr<TopicPublishInfo> topicPublishInfo(weak_topicPublishInfo.lock());
  if (topicPublishInfo) {
    return topicPublishInfo->getMessageQueueList();
  }
  THROW_MQEXCEPTION(MQClientException, "could not find MessageQueue Info of topic: [" + topic + "].", -1);
}

void DefaultMQClient::start() {
  if (getFactory() == NULL) {
    m_clientFactory = MQClientManager::getInstance()->getMQClientFactory(
        getMQClientId(), m_pullThreadNum, m_tcpConnectTimeout, m_tcpTransportTryLockTimeout, m_unitName);
  }
  LOG_INFO(
      "MQClient "
      "start,groupname:%s,clientID:%s,instanceName:%s,nameserveraddr:%s",
      getGroupName().c_str(), getMQClientId().c_str(), getInstanceName().c_str(), getNamesrvAddr().c_str());
}

void DefaultMQClient::shutdown() {
  m_clientFactory->shutdown();
  m_clientFactory = NULL;
}

MQClientFactory* DefaultMQClient::getFactory() const {
  return m_clientFactory;
}

bool DefaultMQClient::isServiceStateOk() {
  return m_serviceState == RUNNING;
}

void DefaultMQClient::setLogLevel(elogLevel inputLevel) {
  ALOG_ADAPTER->setLogLevel(inputLevel);
}

elogLevel DefaultMQClient::getLogLevel() {
  return ALOG_ADAPTER->getLogLevel();
}

void DefaultMQClient::setLogFileSizeAndNum(int fileNum, long perFileSize) {
  ALOG_ADAPTER->setLogFileNumAndSize(fileNum, perFileSize);
}

void DefaultMQClient::setTcpTransportPullThreadNum(int num) {
  if (num > m_pullThreadNum) {
    m_pullThreadNum = num;
  }
}

const int DefaultMQClient::getTcpTransportPullThreadNum() const {
  return m_pullThreadNum;
}

void DefaultMQClient::setTcpTransportConnectTimeout(uint64_t timeout) {
  m_tcpConnectTimeout = timeout;
}
const uint64_t DefaultMQClient::getTcpTransportConnectTimeout() const {
  return m_tcpConnectTimeout;
}

void DefaultMQClient::setTcpTransportTryLockTimeout(uint64_t timeout) {
  if (timeout < 1000) {
    timeout = 1000;
  }
  m_tcpTransportTryLockTimeout = timeout / 1000;
}
const uint64_t DefaultMQClient::getTcpTransportTryLockTimeout() const {
  return m_tcpTransportTryLockTimeout;
}

void DefaultMQClient::setUnitName(string unitName) {
  m_unitName = unitName;
}
const string& DefaultMQClient::getUnitName() const {
  return m_unitName;
}

void DefaultMQClient::setSessionCredentials(const string& input_accessKey,
                                            const string& input_secretKey,
                                            const string& input_onsChannel) {
  m_SessionCredentials.setAccessKey(input_accessKey);
  m_SessionCredentials.setSecretKey(input_secretKey);
  m_SessionCredentials.setAuthChannel(input_onsChannel);
}

const SessionCredentials& DefaultMQClient::getSessionCredentials() const {
  return m_SessionCredentials;
}

//<!************************************************************************
}  // namespace rocketmq
