/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


import type { AsyncCallback, Callback } from './@ohos.base';
import type connection from './@ohos.net.connection';
import { HttpProxy } from './@ohos.net.connection';

type ObserverEventType = 'headerReceive' | 'headersReceive' | 'dataReceive' | 'dataEnd' | 'dataReceiveProgress' | 'dataSendProgress'

export interface HttpResponse {
  result: string | Object | ArrayBuffer;

  resultType: HttpDataType;

  responseCode: ResponseCode | number;

  header: Object;

  cookies: string;

  performanceTiming: PerformanceTiming;
}

export interface HttpRequestOptions {
  method?: RequestMethod;

  extraData?: string | Object | ArrayBuffer;

  expectDataType?: HttpDataType;

  usingCache?: boolean;

  priority?: number;

  header?: Object;

  readTimeout?: number;

  connectTimeout?: number;

  usingProtocol?: HttpProtocol;

  usingProxy?: boolean | HttpProxy;

  caPath?: string;

  resumeFrom?: number;

  resumeTo?: number;

  clientCert?: ClientCert;

  dnsOverHttps?: string;

  dnsServers?: Array<string>;

  maxLimit?: number;

  multiFormDataList?: Array<MultiFormData>;

  certificatePinning?: CertificatePinning | CertificatePinning[];
}

export enum RequestMethod {
  OPTIONS = "OPTIONS",

  GET = "GET",

  HEAD = "HEAD",

  POST = "POST",

  PUT = "PUT",

  DELETE = "DELETE",

  TRACE = "TRACE",

  CONNECT = "CONNECT"
}

export enum ResponseCode {
  OK = 200,

  CREATED,

  ACCEPTED,

  NOT_AUTHORITATIVE,

  NO_CONTENT,

  RESET,

  PARTIAL,

  MULT_CHOICE = 300,

  MOVED_PERM,

  MOVED_TEMP,

  SEE_OTHER,

  NOT_MODIFIED,

  USE_PROXY,

  BAD_REQUEST = 400,

  UNAUTHORIZED,

  PAYMENT_REQUIRED,

  FORBIDDEN,

  NOT_FOUND,

  BAD_METHOD,

  NOT_ACCEPTABLE,

  PROXY_AUTH,

  CLIENT_TIMEOUT,

  CONFLICT,

  GONE,

  LENGTH_REQUIRED,

  PRECON_FAILED,

  ENTITY_TOO_LARGE,

  REQ_TOO_LONG,

  UNSUPPORTED_TYPE,

  RANGE_NOT_SATISFIABLE,

  INTERNAL_ERROR = 500,

  NOT_IMPLEMENTED,

  BAD_GATEWAY,

  UNAVAILABLE,

  GATEWAY_TIMEOUT,

  VERSION
}


export enum HttpProtocol {
  HTTP1_1,

  HTTP2,

  HTTP3
}

export enum HttpDataType {
  STRING,

  OBJECT = 1,

  ARRAY_BUFFER = 2
}



export interface PerformanceTiming {
  dnsTiming: number;

  tcpTiming: number;

  tlsTiming: number;

  firstSendTiming: number;

  firstReceiveTiming: number;

  totalFinishTiming: number;

  redirectTiming: number;

  responseHeaderTiming: number;

  responseBodyTiming: number;

  totalTiming: number;
}

export interface DataReceiveProgressInfo {
  receiveSize: number;
  totalSize: number;
}

export interface DataSendProgressInfo {
  sendSize: number;
  totalSize: number;
}


export interface HttpResponseCache {
  flush(callback: AsyncCallback<void>): void;

  flush(): Promise<void>;

  delete(callback: AsyncCallback<void>): void;

  delete(): Promise<void>;
}

export interface MultiFormData {
  name: string;

  contentType: string;

  remoteFileName?: string;

  data?: string | Object | ArrayBuffer;

  filePath?: string;
}

export enum CertType {
  PEM = 'PEM',

  DER = 'DER',

  P12 = 'P12'
}

export interface ClientCert {
  certPath: string;

  certType?: CertType;

  keyPath: string;

  keyPassword?: string;
}

interface CertificatePinning {
  publicKeyHash: string;
  hashAlgorithm: 'SHA-256';
}

export interface HttpRequest {
  request(url: string, callback: AsyncCallback<HttpResponse>): void;

  request(url: string, options: HttpRequestOptions, callback: AsyncCallback<HttpResponse>): void;

  request(url: string, options?: HttpRequestOptions): Promise<HttpResponse>;

  requestInStream(url: string, callback: AsyncCallback<number>): void;

  requestInStream(url: string, options: HttpRequestOptions, callback: AsyncCallback<number>): void;

  requestInStream(url: string, options?: HttpRequestOptions): Promise<number>;

  destroy(): void;

  on(type: ObserverEventType, callback: AsyncCallback<Object>): void;

  off(type: ObserverEventType, callback?: AsyncCallback<Object>): void;

  on(type: ObserverEventType, callback: Callback<Object>): void;

  off(type: ObserverEventType, callback: Callback<Object>): void;

  once(type: ObserverEventType, callback: Callback<Object>): void;
}

declare namespace http {
  function createHttp(): HttpRequest;
  function createHttpResponseCache(cacheSize?: number): HttpResponseCache;
}

export default http;
