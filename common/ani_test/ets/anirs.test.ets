// Copyright (C) 2025 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

import {Callback, BusinessError, AsyncCallback} from "@ohos.base";

export default namespace ani_test {
    loadLibrary("ani_test")

    export enum EnumNumber {
        ONE = 1,
        TWO = 2,
        THREE = 3,
    }

    export enum EnumString
    {
        ONE = "ONE",
        TWO = "TWO",
        THREE = "THREE",
    }

    export class PrimitiveTest {
        primitiveBool: boolean;
        primitiveI8: byte;
        primitiveI16: short;
        primitiveI32: int;
        primitiveI64: long;
        primitiveF32: float;
        primitiveF64: double;
    }

    export class StructArray
    {
        arrayBool: boolean[] = []
    }

    export enum Action {
      DOWNLOAD,
      UPLOAD
    }

    export class Config {
      action: Action;
      url: string;
    }

    export class ResponseCode {
        code: int;
        url: string;
    }

    export enum HttpProtocol {
        HTTP1_1,
        HTTP2,
        HTTP3
    }

    export native function primitiveTest(input:PrimitiveTest): PrimitiveTest;
    export native function optionBool(input?:boolean): boolean | undefined;
    export native function optionByte(input?:byte): byte | undefined;
    export native function optionShort(input?:short): short | undefined;
    export native function optionInt(input?:int): int | undefined;
    export native function optionLong(input?:long): long | undefined;
    export native function optionDouble(input?:double): double | undefined;

    export native function arrayBool(input: boolean[]): boolean[];
    export native function arrayByte(input: byte[]): byte[];
    export native function arrayShort(input: Array<short>): Array<short>;
    export native function arrayInt(input: int[]): int[];
    export native function arrayLong(input: long[]): long[];
    export native function arrayFloat(input: float[]): float[];
    export native function arrayDouble ( input: double[] ): double[];

    export native function enumTestNumber ( input: EnumNumber): EnumNumber
    export native function enumTestString ( input: EnumString): EnumString
    export native function enumTestStruct(input: ResponseCode | HttpProtocol | int): ResponseCode | HttpProtocol | int

    export native function recordString(input:Record<String,String>) : Record<String, String>
    export native function recordLong(input:Record<Long, Long>) : Record<Long, Long>

    export native function unionTest ( input: boolean | byte | short | int | long | float | double | String | ArrayBuffer | null ): boolean | byte | short | int | long | float | double | String | ArrayBuffer | null;

    export native function arrayBufferTest(input: Uint8Array): Uint8Array;
    export native function uint8ArrayTest(input: Uint8Array): Uint8Array;
    export native function structEnum(input: Config): Config;

    export native function executeCallback1(callback: Callback<int>): void;
    export native function executeCallback2(callback: Callback<int>): void;
    export native function executeCallback3(callback: Callback<int>): void;
    export native function executeCallback4(callback: Callback<int>): void;
    export native function executeAsyncCallback1(callback: AsyncCallback<int>): void;
    export native function executeAsyncCallback2(callback: AsyncCallback<int>): void;
    export native function executeAsyncCallback3(callback: AsyncCallback<int>): void;
    export native function executeAsyncCallback4(callback: AsyncCallback<int>): void;
}

function assertEQ<T, U>(expected: T, actual: U, message?: string): void {
  if (expected !== actual) {
    const errorMessage = message || `Assertion failed: expected ${expected}, but got ${actual}`;
    throw new Error(errorMessage);
  }
}

function main():void{
    primitive_test();
    option_test();
    array_test();
    enum_test();
    record_test();
    union_test();
    array_buffer_test();
    struct_test();
    callback_test();
}

function primitive_test():void {
    let input:ani_test.PrimitiveTest = {
        primitiveBool: true,
        primitiveI8: -128,
        primitiveI16: 32767,
        primitiveI32: -2147483648,
        primitiveI64: 9223372036854775807,
        primitiveF32: 3.13f,
        primitiveF64: 3.14159,
    }
    let output = ani_test.primitiveTest(input);
    
    assertEQ(input.primitiveBool,  output.primitiveBool);
    assertEQ(input.primitiveI8, output.primitiveI8);
    assertEQ(input.primitiveI16, output.primitiveI16);
    assertEQ(input.primitiveI32, output.primitiveI32);
    assertEQ(input.primitiveI64, output.primitiveI64);
    assertEQ(input.primitiveF32, output.primitiveF32);
    assertEQ(input.primitiveF64, output.primitiveF64);
}

function option_test():void {
    let input0 = true;
    let output0 = ani_test.optionBool(input0);
    assertEQ(input0, output0);
    let input1 = undefined;
    let output1 = ani_test.optionBool(input1);
    assertEQ(input1, output1);

    let input2:byte = -128;
    let output2 = ani_test.optionByte( input2 );
    assertEQ(input2, output2);
    let input3 = undefined;
    let output3 = ani_test.optionByte(input3);
    assertEQ( input3, output3 );
    
    let input4:short = 32767;
    let output4 = ani_test.optionShort( input4 );
    assertEQ(input4, output4);
    let input5 = undefined;
    let output5 = ani_test.optionShort(input5);
    assertEQ( input5, output5 );

    let input6:int = -2147483648;
    let output6 = ani_test.optionInt( input6 );
    assertEQ(input6, output6);
    let input7 = undefined;
    let output7 = ani_test.optionInt(input7);
    assertEQ( input7, output7 );

    let input8:long =  9223372036854775807;
    let output8 = ani_test.optionLong( input8 );
    assertEQ(input8, output8);
    let input9 = undefined;
    let output9 = ani_test.optionLong(input9);
    assertEQ( input9, output9 );

    let input10:double =  3.14159;
    let output10 = ani_test.optionDouble( input10 );
    assertEQ(input10, output10);
    let input11 = undefined;
    let output11 = ani_test.optionDouble(input11);
    assertEQ( input11, output11 );
}

function array_test (): void
{
    let input0: boolean[] = [ true, false, true ];
    let output0 = ani_test.arrayBool( input0 );
    assertEQ( JSON.stringify( input0 ), JSON.stringify( output0 ) );
   
    let input1: byte[] = [ 1, 2, 3 ];
    let output1 = ani_test.arrayByte( input1 );
    assertEQ( JSON.stringify(input1), JSON.stringify(output1) );

    let input2: Array<short> = [ 1, 2, 3 ];
    let output2 = ani_test.arrayShort( input2 );
    assertEQ( JSON.stringify(input2), JSON.stringify(output2) );

    let input3: int[] = [ 1, 2, 3 ];
    let output3 = ani_test.arrayInt( input3 );
    assertEQ( JSON.stringify(input3), JSON.stringify(output3) );

    let input4: long[] = [ 1, 2, 3 ];
    let output4 = ani_test.arrayLong( input4 );
    assertEQ( JSON.stringify(input4), JSON.stringify(output4) );

    let input41: float[] = [ 1.0f, 2.0f, 3.0f ];
    let output41 = ani_test.arrayFloat( input41 );
    assertEQ( JSON.stringify( input41 ), JSON.stringify( output41 ) );

    let input5: double[] = [ 1.0, 2.0, 3.0 ];
    let output5 = ani_test.arrayDouble( input5 );
    assertEQ( JSON.stringify( input5 ), JSON.stringify( output5 ) );
    
    let input6: boolean[] = [];
    let output6 = ani_test.arrayBool( input6 );
    assertEQ( JSON.stringify(input6), JSON.stringify(output6) );

    let input7: byte[] = [];
    let output7 = ani_test.arrayByte( input7 );
    assertEQ( JSON.stringify(input7), JSON.stringify(output7) );

    let input8: short[] = [];
    let output8 = ani_test.arrayShort( input8 );
    assertEQ( JSON.stringify(input8), JSON.stringify(output8) );

    let input9: int[] = [];
    let output9 = ani_test.arrayInt( input9 );
    assertEQ( JSON.stringify(input9), JSON.stringify(output9) );

    let input10: long[] = [];
    let output10 = ani_test.arrayLong( input10 );
    assertEQ( JSON.stringify(input10), JSON.stringify(output10) );

    let input11: double[] = [];
    let output11 = ani_test.arrayDouble( input11 );
    assertEQ( JSON.stringify(input11), JSON.stringify(output11) );
}

function enum_test(): void
{
    let input0 = ani_test.EnumNumber.ONE;
    let output0 = ani_test.enumTestNumber( input0 );
    assertEQ( input0, output0 );

    let input1 = ani_test.EnumNumber.TWO;
    let output1 = ani_test.enumTestNumber( input1 );
    assertEQ( input1, output1 );

    let input2 = ani_test.EnumNumber.THREE;
    let output2 = ani_test.enumTestNumber( input2 );
    assertEQ( input2, output2 );

    let input3 = ani_test.EnumString.ONE;
    let output3 = ani_test.enumTestString( input3 );
    assertEQ( input3, output3 );

    let input4 = ani_test.EnumString.TWO;
    let output4 = ani_test.enumTestString( input4 );
    assertEQ( input4, output4 );

    let input5 = ani_test.EnumString.THREE;
    let output5 = ani_test.enumTestString( input5 );
    assertEQ( input5, output5 );

    let intput6: ani_test.ResponseCode = {code: 10, url: "www.baidu.com"};
    let output6 = ani_test.enumTestStruct(intput6)
    assertEQ(JSON.stringify(intput6), JSON.stringify(output6) );
    let intput7 = 666;
    let output7 = ani_test.enumTestStruct(intput7)
    assertEQ(JSON.stringify(intput7), JSON.stringify(output7) );
    let intput8 = ani_test.HttpProtocol.HTTP2;
    let output8 = ani_test.enumTestStruct(intput8)
    assertEQ(intput8, output8 );
}

function record_test (): void
{
    let input:Record<String,String> = { "hello": "rust", "nihao": "ani" };
    let output = ani_test.recordString(input);
    assertEQ( output.get("hello") , "rust" );
    assertEQ( output.get( "nihao" ), "ani" );
    
    let input1: Record<Long, Long> = { 1: 2, 3: 4 };
    let output1 = ani_test.recordLong( input1 );
    assertEQ( output1.get( 1 ), 2 );
    assertEQ( output1.get( 3 ), 4 );
}

function union_test (): void 
{
    let input0:byte = 1;
    let output0 = ani_test.unionTest( input0 );
    assertEQ( input0, output0 );

    let input1:short = 2;
    let output1 = ani_test.unionTest( input1 );
    assertEQ( input1, output1 );

    let input2:int = 3;
    let output2 = ani_test.unionTest( input2 );
    assertEQ( input2, output2 );

    let input3:long = 4;
    let output3 = ani_test.unionTest( input3 );
    assertEQ( input3, output3 );

    let input4:double = 5.0;
    let output4 = ani_test.unionTest( input4 );
    assertEQ( input4, output4 );

    let input5:String = "Hello";
    let output5 = ani_test.unionTest( input5 );
    assertEQ( input5, output5 );

    let input6:boolean = true;
    let output6 = ani_test.unionTest( input6 );
    assertEQ( input6, output6 );

    let input7 = new ArrayBuffer(4);
    let uint8View = new Uint8Array(input7);
    uint8View[0] = 1; // 1*1
    uint8View[1] = 2; // 2*256
    uint8View[2] = 3;
    uint8View[3] = 4;
    let output7 = ani_test.unionTest( input7 ) as ArrayBuffer;
    let uint8ViewOutput = new Uint8Array(output7);
    assertEQ( JSON.stringify(uint8View), JSON.stringify(uint8ViewOutput) );

    let input8:null = null;
    let output8 = ani_test.unionTest( input8 );
    assertEQ( input8, output8 );
}

function array_buffer_test (): void
{
    const buffer = new ArrayBuffer(4);
    const uint8View = new Uint8Array(buffer);
    uint8View[0] = 1; // 1*1
    uint8View[1] = 2; // 2*256
    uint8View[2] = 3;
    uint8View[3] = 4;
    let output = ani_test.arrayBufferTest(uint8View);
    assertEQ(JSON.stringify(uint8View), JSON.stringify(output));
}

function struct_test():void
{
    let input1: ani_test.Config = {action: ani_test.Action.UPLOAD, url: "www.huawei.com"};
    let output1 = ani_test.structEnum(input1);
    assertEQ(JSON.stringify(input1), JSON.stringify(output1));
}

function callback_test():void
{
    let input1: int = 0;
    ani_test.executeCallback1((data: int) => {
        input1 += data;
    })
    assertEQ(input1, 1);

    let input2: int = 0;
    ani_test.executeCallback2((data: int) => {
        input2 += data;
    })
    assertEQ(input2, 2);

    let input3: int = 0;
    ani_test.executeCallback3((data: int) => {
        input3 += data;
    })
    
    await new Promise<int>((resolve) => {
        setTimeout(() => {
            resolve(1)
        }, 500)
    });
    assertEQ(input3, 3);

    // 在devecho上测试，将callback抛到主线程执行
    // let input4: int = 0;
    // ani_test.executeCallback4((data: int) => {
    //     input4 += data;
    // })

    let input5: int = 0;
    ani_test.executeAsyncCallback1((err: BusinessError|null, data: int|undefined) => {
        assertEQ(err?.code, 401);
        if (data) {
            input5 += data;
        }
        
    })
    assertEQ(input5, 1);

    let input6: int = 0;
    ani_test.executeAsyncCallback2((err: BusinessError|null, data: int|undefined) => {
        assertEQ(err?.code, 402);

        if (data) {
            input6 += data;
        }
        
    })
    assertEQ(input6, 2);

    let input7: int = 0;
    ani_test.executeAsyncCallback3((err: BusinessError|null, data: int|undefined) => {
        assertEQ(err?.code, 403);

        if (data) {
            input7 += data;
        }
        
    })
    
    await new Promise<int>((resolve) => {
        setTimeout(() => {
            resolve(1)
        }, 500)
    });
    assertEQ(input7, 3);


    // 在devecho上测试，将callback抛到主线程执行
    // let input8: int = 0;
    // ani_test.executeAsyncCallback4((err: BusinessError, data: int) => {
    //     console.log("[ani_test] executeCallback4 data = %{public}d", data);
    //     assertEQ(err.code, 404);
    //     input8 += data;
    // })
}