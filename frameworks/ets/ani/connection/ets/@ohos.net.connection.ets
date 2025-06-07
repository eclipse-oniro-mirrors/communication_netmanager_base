/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

import type { AsyncCallback } from '@ohos.base';
import { BusinessError } from '@ohos.base'

export default namespace connection {
  loadLibrary("connection_ani") 

  export interface NetHandle {
    netId: number;
  }

  class NetHandleInner implements NetHandle{
    netId: double = 0;
  }

  export native function getDefaultNetSync(): NetHandle;
  
  export function getDefaultNet(callback: AsyncCallback<NetHandle>): void {
    let p1 = taskpool.execute((): NetHandle => {
      return getDefaultNetSync();
    })
    p1.then((content: NullishType) => {
      callback(new BusinessError(), content as NetHandle);
    }, (err: BusinessError): void => {
      callback(err, new NetHandleInner);
    });
  }

  export function getDefaultNet(): Promise<NetHandle> {
    return new Promise<NetHandle>((resolve, reject) => {
      taskpool.execute((): NetHandle => {
        return getDefaultNetSync();
      }).then((content: NullishType) => {
        resolve(content as NetHandle);
      }, (err: BusinessError): void => {
        reject(err);
      });
    });
  }

}