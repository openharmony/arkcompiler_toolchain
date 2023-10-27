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

#include "tooling/client/domain/heapprofiler_client.h"
#include "common/log_wrapper.h"
#include "tooling/client/utils/utils.h"

#include <map>
#include <functional>
#include <cstring>

using Result = panda::ecmascript::tooling::Result;
namespace OHOS::ArkCompiler::Toolchain {
static constexpr int32_t SAMPLING_INTERVAL = 16384;
bool HeapProfilerClient::DispatcherCmd(int id, const std::string &cmd, const std::string &arg, std::string* reqStr)
{
    if (reqStr == nullptr) {
        return false;
    }
    path_ = arg;

    std::map<std::string, std::function<std::string()>> dispatcherTable {
        { "allocationtrack", std::bind(&HeapProfilerClient::AllocationTrackCommand, this, id)},
        { "allocationtrack-stop", std::bind(&HeapProfilerClient::AllocationTrackStopCommand, this, id)},
        { "heapdump", std::bind(&HeapProfilerClient::HeapDumpCommand, this, id)},
        { "heapprofiler-enable", std::bind(&HeapProfilerClient::Enable, this, id)},
        { "heapprofiler-disable", std::bind(&HeapProfilerClient::Disable, this, id)},
        { "sampling", std::bind(&HeapProfilerClient::Samping, this, id)},
        { "sampling-stop", std::bind(&HeapProfilerClient::SampingStop, this, id)},
        { "collectgarbage", std::bind(&HeapProfilerClient::CollectGarbage, this, id)}
    };

    auto entry = dispatcherTable.find(cmd);
    if (entry != dispatcherTable.end() && entry->second != nullptr) {
        *reqStr = entry->second();
        LOGI("DispatcherCmd reqStr1: %{public}s", reqStr->c_str());
        return true;
    }

    *reqStr = "Unknown commond: " + cmd;
    LOGI("DispatcherCmd reqStr2: %{public}s", reqStr->c_str());
    return false;
}

std::string HeapProfilerClient::HeapDumpCommand(int id)
{
    idEventMap_.emplace(id, HEAPDUMP);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.takeHeapSnapshot");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("reportProgress", true);
    params->Add("captureNumericValue", true);
    params->Add("exposeInternals", false);
    request->Add("params", params);
    return request->Stringify();
}

std::string HeapProfilerClient::AllocationTrackCommand(int id)
{
    idEventMap_.emplace(id, ALLOCATION);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.startTrackingHeapObjects");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("trackAllocations", true);
    request->Add("params", params);
    return request->Stringify();
}

std::string HeapProfilerClient::AllocationTrackStopCommand(int id)
{
    idEventMap_.emplace(id, ALLOCATION_STOP);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.stopTrackingHeapObjects");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("reportProgress", true);
    request->Add("params", params);
    return request->Stringify();
}

std::string HeapProfilerClient::Enable(int id)
{
    idEventMap_.emplace(id, ENABLE);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.enable");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);
    return request->Stringify();
}

std::string HeapProfilerClient::Disable(int id)
{
    idEventMap_.emplace(id, DISABLE);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.disable");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);
    return request->Stringify();
}

std::string HeapProfilerClient::Samping(int id)
{
    idEventMap_.emplace(id, SAMPLING);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.startSampling");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("samplingInterval", SAMPLING_INTERVAL);
    request->Add("params", params);
    return request->Stringify();
}

std::string HeapProfilerClient::SampingStop(int id)
{
    idEventMap_.emplace(id, SAMPLING_STOP);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.stopSampling");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);
    return request->Stringify();
}

std::string HeapProfilerClient::CollectGarbage(int id)
{
    idEventMap_.emplace(id, COLLECT_GARBAGE);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.collectGarbage");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);
    return request->Stringify();
}

void HeapProfilerClient::RecvReply(std::unique_ptr<PtJson> json)
{
    if (json == nullptr) {
        LOGE("json parse error");
        return;
    }

    if (!json->IsObject()) {
        LOGE("json parse format error");
        json->ReleaseRoot();
        return;
    }

    Result ret;
    std::string wholeMethod;
    std::string method;
    ret = json->GetString("method", &wholeMethod);
    if (ret != Result::SUCCESS) {
        LOGE("find method error");
        return;
    }

    std::string::size_type length = wholeMethod.length();
    std::string::size_type indexPoint = 0;
    indexPoint = wholeMethod.find_first_of('.', 0);
    if (indexPoint == std::string::npos || indexPoint == 0 || indexPoint == length - 1) {
        return;
    }
    method = wholeMethod.substr(indexPoint + 1, length);
    if (method == "lastSeenObjectId") {
        isAllocationMsg_ = true;
    }

    std::unique_ptr<PtJson> params;
    ret = json->GetObject("params", &params);
    if (ret != Result::SUCCESS) {
        LOGE("find params error");
        return;
    }

    std::string chunk;
    ret = params->GetString("chunk", &chunk);
    if (ret != Result::SUCCESS) {
        LOGE("find chunk error");
        return;
    }

    std::string head = "{\"snapshot\":\n";
    if (!strncmp(chunk.c_str(), head.c_str(), head.length())) {
        char date[16];
        char time[16];
        bool res = Utils::GetCurrentTime(date, time, sizeof(date));
        if (!res) {
            LOGE("arkdb: get time failed");
            return;
        }
        if (isAllocationMsg_) {
            fileName_ = "/data/Heap-" + std::string(date) + "T" + std::string(time) + ".heaptimeline";
            std::cout << "heaptimeline file name is " << fileName_ << std::endl;
        } else {
            fileName_ = "/data/Heap-" + std::string(date) + "T" + std::string(time) + ".heapsnapshot";
            std::cout << "heapsnapshot file name is " << fileName_ << std::endl;
        }
        std::cout << ">>> ";
        fflush(stdout);
    }

    std::string tail = "]\n}\n";
    std::string subStr = chunk.substr(chunk.length() - tail.length(), chunk.length());
    if (!strncmp(subStr.c_str(), tail.c_str(), tail.length())) {
        isAllocationMsg_ = false;
    }
    WriteHeapProfilerForFile(fileName_, chunk);
}

bool HeapProfilerClient::WriteHeapProfilerForFile(const std::string &fileName, const std::string &data)
{
    std::ofstream ofs;
    std::string pathname = path_ + fileName;
    std::string realPath;
    bool res = Utils::RealPath(pathname, realPath, false);
    if (!res) {
        LOGE("arkdb: path is not realpath!");
        return false;
    }
    ofs.open(pathname.c_str(), std::ios::app);
    if (!ofs.is_open()) {
        LOGE("arkdb: file open error!");
        return false;
    }
    size_t strSize = data.size();
    ofs.write(data.c_str(), strSize);
    ofs.close();
    ofs.clear();
    return true;
}
} // OHOS::ArkCompiler::Toolchain