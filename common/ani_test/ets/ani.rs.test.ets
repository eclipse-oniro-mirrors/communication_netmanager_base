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

import {Callback, BusinessError} from "@ohos.base";

enum EnumNumber{
    ONE = 1,
    TWO = 2,
    THREE = 3,
}

enum EnumString
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

export default namespace ani_test {
    loadLibrary("ani_test")
    
    export native function PrimitiveTest(input:PrimitiveTest): PrimitiveTest;
    export native function OptionBool(input?:boolean): boolean | undefined;
    export native function OptionByte(input?:byte): byte | undefined;
    export native function OptionShort(input?:short): short | undefined;
    export native function OptionInt(input?:int): int | undefined;
    export native function OptionLong(input?:long): long | undefined;
    export native function OptionDouble(input?:double): double | undefined;

    export native function ArrayBool(input: boolean[]): boolean[];
    export native function ArrayByte(input: byte[]): byte[];
    export native function ArrayShort(input: Array<short>): Array<short>;
    export native function ArrayInt(input: int[]): int[];
    export native function ArrayLong(input: long[]): long[];
    export native function ArrayFloat(input: float[]): float[];
    export native function ArrayDouble ( input: double[] ): double[];

    export native function EnumTestNumber ( input: EnumNumber ): EnumNumber;
    export native function EnumTestString ( input: EnumString ): EnumString;

    export native function StructArrayTest (input: StructArray ): StructArray;

    export native function RecordString(input:Record<String,String>) : Record<String, String>
    export native function RecordLong(input:Record<Long, Long>) : Record<Long, Long>

    export native function UnionTest ( input: boolean | byte | short | int | long | float | double | String | ArrayBuffer | null ): boolean | byte | short | int | long | float | double | String | ArrayBuffer | null;

    export native function ArrayBufferTest(input: Uint8Array): Uint8Array;
    export native function Uint8ArrayTest(input: Uint8Array): Uint8Array;

}

function main():void{
    primitive_test();
    // option_test();
    // array_test();
    // enum_test();
    // record_test();
    // union_test();
    // array_buffer_test();
}

function primitive_test():void {
    let input:PrimitiveTest = {
        primitiveBool: true,
        primitiveI8: -128,
        primitiveI16: 32767,
        primitiveI32: -2147483648,
        primitiveI64: 9223372036854775807,
        primitiveF32: 3.13,
        primitiveF64: 3.14159,
    }
    let output = ani_test.PrimitiveTest(input);
    assertEQ(input.primitiveBool, output.primitiveBool);
    assertEQ(input.primitiveI8, output.primitiveI8);
    assertEQ(input.primitiveI16, output.primitiveI16);
    assertEQ(input.primitiveI32, output.primitiveI32);
    assertEQ(input.primitiveI64, output.primitiveI64);
    assertEQ(input.primitiveF32, output.primitiveF32);
    assertEQ(input.primitiveF64, output.primitiveF64);
}

function option_test():void {
    let input0 = true;
    let output0 = ani_test.OptionBool(input0);
    assertEQ(input0, output0);
    let input1 = undefined;
    let output1 = ani_test.OptionBool(input1);
    assertEQ(input1, output1);

    let input2:byte = -128;
    let output2 = ani_test.OptionByte( input2 );
    assertEQ(input2, output2);
    let input3 = undefined;
    let output3 = ani_test.OptionByte(input3);
    assertEQ( input3, output3 );
    
    let input4:short = 32767;
    let output4 = ani_test.OptionShort( input4 );
    assertEQ(input4, output4);
    let input5 = undefined;
    let output5 = ani_test.OptionShort(input5);
    assertEQ( input5, output5 );

    let input6:int = -2147483648;
    let output6 = ani_test.OptionInt( input6 );
    assertEQ(input6, output6);
    let input7 = undefined;
    let output7 = ani_test.OptionInt(input7);
    assertEQ( input7, output7 );

    let input8:long =  9223372036854775807;
    let output8 = ani_test.OptionLong( input8 );
    assertEQ(input8, output8);
    let input9 = undefined;
    let output9 = ani_test.OptionLong(input9);
    assertEQ( input9, output9 );

    let input10:double =  3.14159;
    let output10 = ani_test.OptionDouble( input10 );
    assertEQ(input10, output10);
    let input11 = undefined;
    let output11 = ani_test.OptionDouble(input11);
    assertEQ( input11, output11 );
}

function array_test (): void
{
    let input0: boolean[] = [ true, false, true ];
    let output0 = ani_test.ArrayBool( input0 );
    assertEQ( JSON.stringify( input0 ), JSON.stringify( output0 ) );
   
    let input1: byte[] = [ 1, 2, 3 ];
    let output1 = ani_test.ArrayByte( input1 );
    assertEQ( JSON.stringify(input1), JSON.stringify(output1) );

    let input2: Array<short> = [ 1, 2, 3 ];
    let output2 = ani_test.ArrayShort( input2 );
    assertEQ( JSON.stringify(input2), JSON.stringify(output2) );

    let input3: int[] = [ 1, 2, 3 ];
    let output3 = ani_test.ArrayInt( input3 );
    assertEQ( JSON.stringify(input3), JSON.stringify(output3) );

    let input4: long[] = [ 1, 2, 3 ];
    let output4 = ani_test.ArrayLong( input4 );
    assertEQ( JSON.stringify(input4), JSON.stringify(output4) );

    let input41: float[] = [ 1.0, 2.0, 3.0 ];
    let output41 = ani_test.ArrayFloat( input41 );
    assertEQ( JSON.stringify( input41 ), JSON.stringify( output41 ) );

    let input5: double[] = [ 1.0, 2.0, 3.0 ];
    let output5 = ani_test.ArrayDouble( input5 );
    assertEQ( JSON.stringify( input5 ), JSON.stringify( output5 ) );
    
    let input6: boolean[] = [];
    let output6 = ani_test.ArrayBool( input6 );
    assertEQ( JSON.stringify(input6), JSON.stringify(output6) );

    let input7: byte[] = [];
    let output7 = ani_test.ArrayByte( input7 );
    assertEQ( JSON.stringify(input7), JSON.stringify(output7) );

    let input8: short[] = [];
    let output8 = ani_test.ArrayShort( input8 );
    assertEQ( JSON.stringify(input8), JSON.stringify(output8) );

    let input9: int[] = [];
    let output9 = ani_test.ArrayInt( input9 );
    assertEQ( JSON.stringify(input9), JSON.stringify(output9) );

    let input10: long[] = [];
    let output10 = ani_test.ArrayLong( input10 );
    assertEQ( JSON.stringify(input10), JSON.stringify(output10) );

    let input11: double[] = [];
    let output11 = ani_test.ArrayDouble( input11 );
    assertEQ( JSON.stringify(input11), JSON.stringify(output11) );
}

function enum_test(): void
{
    let input0 = EnumNumber.ONE;
    let output0 = ani_test.EnumTestNumber( input0 );
    assertEQ( input0, output0 );

    let input1 = EnumNumber.TWO;
    let output1 = ani_test.EnumTestNumber( input1 );
    assertEQ( input1, output1 );

    let input2 = EnumNumber.THREE;
    let output2 = ani_test.EnumTestNumber( input2 );
    assertEQ( input2, output2 );

    let input3 = EnumString.ONE;
    let output3 = ani_test.EnumTestString( input3 );
    assertEQ( input3, output3 );

    let input4 = EnumString.TWO;
    let output4 = ani_test.EnumTestString( input4 );
    assertEQ( input4, output4 );

    let input5 = EnumString.THREE;
    let output5 = ani_test.EnumTestString( input5 );
    assertEQ( input5, output5 );
}

function record_test (): void
{
    let input:Record<String,String> = { "hello": "rust", "nihao": "ani" };
    let output = ani_test.RecordString(input);
    assertEQ( output.get("hello") , "rust" );
    assertEQ( output.get( "nihao" ), "ani" );
    
    let input1: Record<Long, Long> = { 1: 2, 3: 4 };
    let output1 = ani_test.RecordLong( input1 );
    assertEQ( output1.get( 1 ), 2 );
    assertEQ( output1.get( 3 ), 4 );
}

function union_test (): void 
{
    let input0:byte = 1;
    let output0 = ani_test.UnionTest( input0 );
    assertEQ( input0, output0 );

    let input1:short = 2;
    let output1 = ani_test.UnionTest( input1 );
    assertEQ( input1, output1 );

    let input2:int = 3;
    let output2 = ani_test.UnionTest( input2 );
    assertEQ( input2, output2 );

    let input3:long = 4;
    let output3 = ani_test.UnionTest( input3 );
    assertEQ( input3, output3 );

    let input4:double = 5.0;
    let output4 = ani_test.UnionTest( input4 );
    assertEQ( input4, output4 );

    let input5:String = "Hello";
    let output5 = ani_test.UnionTest( input5 );
    assertEQ( input5, output5 );

    let input6:boolean = true;
    let output6 = ani_test.UnionTest( input6 );
    assertEQ( input6, output6 );

    let input7 = new ArrayBuffer(4);
    let uint8View = new Uint8Array(input7);
    uint8View[0] = 1; // 1*1
    uint8View[1] = 2; // 2*256
    uint8View[2] = 3;
    uint8View[3] = 4;
    let output7 = ani_test.UnionTest( input7 ) as ArrayBuffer;
    let uint8ViewOutput = new Uint8Array(output7);
    assertEQ( JSON.stringify(uint8View), JSON.stringify(uint8ViewOutput) );

    let input8:null = null;
    let output8 = ani_test.UnionTest( input8 );
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
    let output = ani_test.ArrayBufferTest(uint8View);
    assertEQ(JSON.stringify(uint8View), JSON.stringify(output));
}

