/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef ECMASCRIPT_TOOLING_CLIENT_MANAGER_STACK_MANAGER_H
#define ECMASCRIPT_TOOLING_CLIENT_MANAGER_STACK_MANAGER_H

#include <map>
#include <variant>

#include "log_wrapper.h"
#include "pt_json.h"
#include "pt_types.h"

using PtJson = panda::ecmascript::tooling::PtJson;
using Result = panda::ecmascript::tooling::Result;
using CallFrame = panda::ecmascript::tooling::CallFrame;
using Scope = panda::ecmascript::tooling::Scope;
namespace OHOS::ArkCompiler::Toolchain {
class StackManager final {
public:
    static StackManager& GetInstance();

    std::map<int32_t, std::map<int32_t, std::string>> GetScopeChainInfo();
    void SetCallFrames(std::map<int32_t, std::unique_ptr<CallFrame>> callFrames);
    void ClearCallFrame();
    void ShowCallFrames();
    void PrintScopeChainInfo(const std::map<int32_t, std::map<int32_t, std::string>>& scopeInfos);

private:
    static StackManager instance_;
    std::map<int32_t, std::unique_ptr<CallFrame>> callFrames_ {};
    StackManager() = default;
    StackManager(const StackManager&) = delete;
    StackManager& operator=(const StackManager&) = delete;
};
} // OHOS::ArkCompiler::Toolchain
#endif