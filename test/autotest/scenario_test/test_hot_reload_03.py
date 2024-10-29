#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright (c) 2024 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description: Scenario test case.
"""

import logging
import os
import time

import pytest

from aw import Application
from aw import Utils


@pytest.mark.hot_reload
@pytest.mark.timeout(180)
class TestHotReload03:
    """
    测试用例：热重载_重载页面未启动
    测试步骤：
        1、启动应用，加载Index页面
        2、点击进入Mine页面，点击屏幕，输出日志：
            Test Application HotReloadPages01::Mine => add(): 3
        3、触发热重载，输出日志：
            Test Application HotReloadPages01::Mine => add(): 46
        4、返回Index页面，重新进入Mine页面，点击屏幕，输出日志：
            Test Application HotReloadPages01::Mine => add(): 46
        4、在Mine页面再次触发热重载，输出日志：
            Test Application HotReloadPages01::Mine => add(): 134

    测试应用代码示例：
        修改前：
            Index.ets
                .fontSize(getFontSize())
                .onClick(() => {
                  router.pushUrl({'url': 'pages/Mine'})
                })
                function getFontSize() {
                  let fontSize = 50
                  console.log("Test Application HotReloadPages01::Index => fontSize:", fontSize);
                  return fontSize
                }
            Mine.ets
                .onClick(() => {
                  console.log("Test Application HotReloadPages01::Mine => add():", add(1, 2));
                })
        第一次修改：HotReloadPages01.03.hqf
            Mine.ets
                .onClick(() => {
                  console.log("Test Application HotReloadPages01::Mine => add():", add(12, 34));
                })
        第二次修改：HotReloadPages01.04.hqf
            Mine.ets
                .onClick(() => {
                  console.log("Test Application HotReloadPages01::Mine => add():", add(56, 78));
                })
    """

    def setup_method(self):
        logging.info('Start running test_hot_reload_03: setup')

        self.log_path = rf'{os.path.dirname(__file__)}\..\log'
        self.hilog_file_name = 'test_hot_reload_03.hilog.txt'
        self.hilog_file_path = os.path.join(self.log_path, self.hilog_file_name)
        self.id_generator = Utils.message_id_generator()

        # receive the hilog before the test start
        Utils.clear_fault_log()
        self.hilog_process, self.write_thread = Utils.save_hilog(log_path=self.log_path,
                                                                 file_name=self.hilog_file_name,
                                                                 debug_on=False)

    def teardown_method(self):
        Application.uninstall(self.config['bundle_name'])

        # terminate the hilog receive process after the test done
        time.sleep(3)
        self.hilog_process.stdout.close()
        self.hilog_process.terminate()
        self.hilog_process.wait()
        self.write_thread.join()

        Utils.save_fault_log(log_path=self.log_path)
        logging.info('test_hot_reload_03 done')

    def test(self, test_suite_hotreload_pages_01):
        logging.info('Start running test_hot_reload_03: test')
        self.config = test_suite_hotreload_pages_01
        ################################################################################################################
        # jump to the Mine page
        ################################################################################################################
        Application.click_on_middle()
        time.sleep(3)
        Application.click_on_middle()   # trigger onclick event
        time.sleep(3)
        matched_log = Utils.search_hilog(self.hilog_file_path,
                                         key_world=b"Test Application HotReloadPages01::Mine => add(): 3")
        logging.info(matched_log)
        assert len(matched_log) == 1
        ################################################################################################################
        # 1st hot reload
        ################################################################################################################
        logging.info(f'{"=" * 30} 1st Hot Reload {"=" * 30}')
        Utils.hdc_file_send(source=self.config['local_hqf_03_path'], sink=self.config['remote_hqf_03_path'])
        Application.hot_reload(self.config['remote_hqf_03_path'])
        time.sleep(3)
        Application.click_on_middle()    # trigger onclick event
        time.sleep(3)
        matched_log = Utils.search_hilog(self.hilog_file_path,
                                         key_world=b"Test Application HotReloadPages01::Mine => add(): 46")
        logging.info(matched_log)
        assert len(matched_log) == 1
        ################################################################################################################
        # back to the Index page, the re-enter the Mine page
        ################################################################################################################
        logging.info(f'{"=" * 30} Back and re-enter the Mine page {"=" * 30}')
        Application.back()
        time.sleep(3)
        Application.click_on_middle()  # jump to the Mine page
        time.sleep(3)
        Application.click_on_middle()  # trigger onclick event
        time.sleep(3)
        matched_log = Utils.search_hilog(self.hilog_file_path,
                                         key_world=b"Test Application HotReloadPages01::Mine => add(): 46")
        logging.info(matched_log)
        assert len(matched_log) == 2    # Counting the previous one.
        ################################################################################################################
        # 2nd hot reload
        ################################################################################################################
        logging.info(f'{"=" * 30} 2nd Hot Reload {"=" * 30}')
        Utils.hdc_file_send(source=self.config['local_hqf_04_path'], sink=self.config['remote_hqf_04_path'])
        Application.hot_reload(self.config['remote_hqf_04_path'])
        time.sleep(3)
        Application.click_on_middle()    # trigger onclick event
        time.sleep(3)
        matched_log = Utils.search_hilog(self.hilog_file_path,
                                         key_world=b"Test Application HotReloadPages01::Mine => add(): 134")
        logging.info(matched_log)
        assert len(matched_log) == 1
        ################################################################################################################
        # check if the application is running normally
        ################################################################################################################
        pid = Application.get_pid(self.config['bundle_name'])
        assert pid == self.config['pid'], logging.error(f'App is no longer running with pid: {pid}')
