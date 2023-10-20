/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef ECMASCRIPT_TOOLING_TEST_UTILS_TEST_UTIL_H
#define ECMASCRIPT_TOOLING_TEST_UTILS_TEST_UTIL_H

#include "tooling/test/client_utils/test_actions.h"

#include "tooling/client/manager/domain_manager.h"
#include "tooling/client/websocket/websocket_client.h"
#include "ecmascript/jspandafile/js_pandafile_manager.h"
#include "ecmascript/debugger/js_debugger.h"
#include "os/mutex.h"

namespace panda::ecmascript::tooling::test {
template<class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>>
using CUnorderedMap = panda::ecmascript::CUnorderedMap<Key, T, Hash, KeyEqual>;
using TestMap = CUnorderedMap<std::string, std::unique_ptr<TestActions>>;
using namespace OHOS::ArkCompiler::Toolchain;

#ifdef OHOS_PLATFORM
#define DEBUGGER_LIBRARY "libark_debugger.z.so"
#else
#define DEBUGGER_LIBRARY "libark_debugger.so"
#endif

class TestUtil {
public:
    static void RegisterTest(const std::string &testName, std::unique_ptr<TestActions> test)
    {
        testMap_.insert({testName, std::move(test)});
    }

    static TestActions *GetTest(const std::string &name)
    {
        auto iter = std::find_if(testMap_.begin(), testMap_.end(), [&name](auto &it) {
            return it.first == name;
        });
        if (iter != testMap_.end()) {
            return iter->second.get();
        }
        LOG_DEBUGGER(FATAL) << "Test " << name << " not found";
        return nullptr;
    }

    static TestMap &GetTests()
    {
        return testMap_;
    }

    static void ForkSocketClient(int port, const std::string &name);

private:
    static void NotifyFail(int cmdId, DomainManager &domainManager, WebsocketClient &client);
    static void NotifySuccess(int cmdId, DomainManager &domainManager, WebsocketClient &client);

    static TestMap testMap_;
};
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_UTILS_TEST_UTIL_H
