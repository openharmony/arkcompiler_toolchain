/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

function smart_func1() {
    var a = 1;
    var b = 2;
    var c = 3;
    print("smart_func1");
}

function smart_func2() {
    var j = 1;
    var k = 2;
    var l = 3;
    print("smart_func2");
}

function test_func() {
    smart_func1();smart_func2();
}

test_func();