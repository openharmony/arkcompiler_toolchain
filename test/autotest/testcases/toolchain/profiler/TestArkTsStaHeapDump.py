#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

#==================================================================
#文 件 名：                 TestArkTsStaHeapDump.py
#文件说明：                 静态ArkTS heapdump测试用例
#==================================================================
注意： 该用例执行前需要设置
    hdc shell param set persist.ark.enableDebugMode true
    hdc shell reboot
测试步骤：
    1. 清空heapdump存放路径
    2. -D 拉起应用
    3. 发送相关协议，进入应用首页
    4. 发送ArkTS sta heapdump协议触发heapdump
    5. 检查heapdump存放路径，路径下成功生成了对应的snapshot文件，且文件大小不为0
#!!================================================================
"""
import sys
from pathlib import Path

root_path = Path(__file__).parent.parent.parent.parent
resource_path = root_path / 'resource'
sys.path.append(str(root_path / 'aw'))  # add aw path to sys.path

from devicetest.core.test_case import TestCase, Step
from hypium import UiDriver
from all_utils import CommonUtils, UiUtils
from cdp import debugger, runtime
from implement_api import debugger_api, runtime_api, heap_profiler_api


class TestArkTsStaHeapDump(TestCase):
    def __init__(self, controllers):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, controllers)
        self.driver = UiDriver(self.device1)
        self.ui_utils = UiUtils(self.driver)
        self.common_utils = CommonUtils(self.driver)
        self.id_generator = CommonUtils.message_id_generator()
        self.config = {
            'start_mode': '-D',
            'connect_server_port': 15658,
            'debugger_server_port': 15659,
            'bundle_name': 'com.example.arktsstaticheapdump',
            'hap_name': 'ArkTsStaticHeapdump.hap',
            'hap_path': str(resource_path / 'hap' / 'ArkTsStaticHeapdump.hap'),
            'file_path': {
                'entry_ability': r'entry\src\main\ets\entryability\EntryAbility.ets',
                'index': r'entry\src\main\ets\pages\Index.ets',
            }
        }

    def setup(self):
        Step('1.安装应用')
        self.driver.install_app(self.config['hap_path'], "-r")
        Step('2.启动应用')
        self.driver.start_app(package_name=self.config['bundle_name'], params=self.config['start_mode'])
        self.config['pid'] = self.common_utils.get_pid(self.config['bundle_name'])
        assert self.config['pid'] != 0, f'Failed to get pid of {self.config["bundle_name"]}'
        Step('3.设置屏幕常亮')
        self.ui_utils.keep_awake()
        Step('4.端口映射，连接server')
        self.common_utils.connect_server(self.config)
        self.debugger_impl = debugger_api.DebuggerImpl(self.id_generator, self.config['websocket'])
        self.runtime_impl = runtime_api.RuntimeImpl(self.id_generator, self.config['websocket'])
        self.heap_profiler_impl = heap_profiler_api.HeapProfilerImpl(self.id_generator, self.config['websocket'])

    def process(self):
        Step('5.执行测试用例')
        websocket = self.config['websocket']
        taskpool = self.config['taskpool']
        taskpool.submit(websocket.main_task(taskpool, self.test, self.config['pid']))
        taskpool.await_taskpool()
        taskpool.task_join()
        if taskpool.task_exception:
            raise taskpool.task_exception

    def teardown(self):
        Step('6.关闭应用')
        self.driver.stop_app(self.config['bundle_name'])
        Step('7.卸载应用')
        self.driver.uninstall_app(self.config['bundle_name'])

    async def test(self, websocket):
        ################################################################################################################
        # Clear the snapshot files in /data/log/faultlog/temp/
        ################################################################################################################
        self.common_utils.remove_file('/data/log/faultlog/temp/jsheap-*')
        ################################################################################################################
        # main thread: connect the debugger server
        ################################################################################################################
        main_thread = await websocket.connect_to_debugger_server(self.config['pid'], True, True)
        ################################################################################################################
        # main thread: Runtime.enable
        ################################################################################################################
        await self.runtime_impl.send("Runtime.enable", main_thread)
        await self.runtime_impl.send("Runtime.enableStatic", main_thread, None, True, 2)
        ################################################################################################################
        # main thread: Debugger.enable
        ################################################################################################################
        await self.debugger_impl.send("Debugger.enable", main_thread)
        await self.debugger_impl.send("Debugger.enableStatic", main_thread, None, True, 1)
        ################################################################################################################
        # main thread: Runtime.runIfWaitingForDebugger
        ################################################################################################################
        await self.runtime_impl.send("Runtime.runIfWaitingForDebugger", main_thread, None, True, 1)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['entry_ability'])
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['entry_ability'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resumeStatic", main_thread, None, True, 1)
        ################################################################################################################
        # main thread: Debugger.scriptParsed
        ################################################################################################################
        response = await self.debugger_impl.recv("Debugger.scriptParsed", main_thread)
        self.common_utils.assert_equal(response['params']['url'], self.config['file_path']['index'])
        response = await self.debugger_impl.recv("Debugger.paused", main_thread)
        self.common_utils.assert_equal(response['params']['callFrames'][0]['url'],
                                       self.config['file_path']['index'])
        self.common_utils.assert_equal(response['params']['reason'], 'Break on start')
        ################################################################################################################
        # main thread: Debugger.resume
        ################################################################################################################
        await self.debugger_impl.send("Debugger.resumeStatic", main_thread, None, True, 1)
        ################################################################################################################
        # main thread: HeapProfiler.takeHeapSnapshotStatic
        ################################################################################################################
        await self.heap_profiler_impl.send("HeapProfiler.takeHeapSnapshotStatic", main_thread)
        ################################################################################################################
        # Check the snapshot file.
        ################################################################################################################
        self.common_utils.check_snapshot('/data/log/faultlog/temp', self.config['pid'])
        ################################################################################################################
        # close the websocket connections
        ################################################################################################################
        await websocket.send_msg_to_debugger_server(main_thread.instance_id, main_thread.send_msg_queue, 'close')
        await websocket.send_msg_to_connect_server('close')
        ################################################################################################################