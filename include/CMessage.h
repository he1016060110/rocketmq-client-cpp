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

#ifndef __C_MESSAGE_H__
#define __C_MESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

//typedef struct _CMessage_ CMessage;
struct CMessage;


CMessage *CreateMessage(const char *topic);
int DestroyMessage(CMessage *msg);
int SetMessageTopic(CMessage *msg, const char *topic);
int SetMessageTags(CMessage *msg, const char *tags);
int SetMessageKeys(CMessage *msg, const char *keys);
int SetMessageBody(CMessage *msg, const char *body);
int SetByteMessageBody(CMessage *msg, const char *body, int len);
int SetMessageProperty(CMessage *msg, const char *key, const char *value);

#ifdef __cplusplus
};
#endif
#endif //__C_MESSAGE_H__
