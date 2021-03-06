#
# Copyright (C) 2018 ETH Zurich and University of Bologna
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Authors: Germain Haugou, ETH (germain.haugou@iis.ee.ethz.ch)
 
import vp.time_domain
import vp.trace_engine
import vp_core
import plptree

from plp_platform import *
import plp_flash_stimuli


class Runner(Platform):

    def __init__(self, config, tree):

        self.config = config
        super(Runner, self).__init__(self.config, tree)

        systemConfig = self.config.args.config
        if systemConfig is not None:
            self.system_tree = plptree.get_configs_from_env(
                self.config.args.configDef, systemConfig)
        else:
            self.system_tree = plptree.get_configs_from_file(
                self.config.args.configFile)[0]



    def get_flash_preload_file(self):
        return os.path.join(os.getcwd(), 'flash_preload_file.bin')


    def prepare(self):
        comps = []
        comps_conf = self.get_json().get('**/fs/files')
        if comps_conf is not None:
            comps = comps_conf.get_dict()


        if comps_conf is not None or self.get_json().get_child_bool('**/runner/boot_from_flash'):

            if plp_flash_stimuli.genFlashImage(
                raw_stim=self.get_flash_preload_file(),
                bootBinary=self.get_json().get('**/loader/binaries').get_elem(0).get(),
                comps=comps,
                verbose=self.tree.get('**/runner/verbose').get(),
                archi=self.tree.get('**/pulp_chip_family').get(),
                flashType=self.tree.get('**/runner/flash_type').get()):
                return -1

        return 0

    def run(self):

        autorun_conf = self.tree.get('**/debug_bridge/autorun')
        if autorun_conf is not None and autorun_conf.get_bool() and not self.config.getOption('reentrant'):
            os.execlp('pulp-run-bridge', '--dir=%s' % self.config.getOption('dir'), '--config-file=%s' % self.config.getOption('configFile'))

        system = self.system_tree
        if self.system_tree.get_config('system') is not None:
          system = self.system_tree.get_config('system_tree')

        for config_opt in self.config.getOption('configOpt'):
            key, value = config_opt.split(':')
            self.system_tree.set(key, value)
            self.get_json().user_set(key, value)


        autorun = self.tree.get('**/debug_bridge/autorun')
        bridge_active = self.tree.get('**/debug_bridge/active')

        bridge = autorun is not None and autorun.get_bool() or \
          self.get_json().get('**/gdb/active').get_bool() or \
          bridge_active is not None and bridge_active.get_bool()
        
        if bridge:
            self.get_json().get('**/jtag_proxy').set('active', True)
            self.get_json().get('gvsoc').set('use_external_bridge', True)

        if (autorun is None or not autorun.get()) and self.get_json().get_child_str('**/loader/boot/mode') != 'bridge':
            binaries = self.get_json().get('**/loader/binaries').get_dict()
            for binary in binaries:
                self.get_json().get('**/plt_loader').set('binaries', binary)


        if self.get_json().get_child_str('**/loader/boot/mode') != 'rom' and not bridge:
            set_pc_addr = self.get_json().get_child_int('**/loader/set_pc_addr')
            if set_pc_addr != None:
                self.get_json().get('**/plt_loader').set('set_pc_addr', '0x%x' % set_pc_addr)
            set_pc_offset = self.get_json().get_child_str('**/loader/set_pc_offset')
            if set_pc_offset != None:
                self.get_json().get('**/plt_loader').set('set_pc_offset', set_pc_offset)
            start_addr = self.get_json().get_child_int('**/loader/start_addr')
            if start_addr != None:
                self.get_json().get('**/plt_loader').set('start_addr', '0x%x' % start_addr)
            start_value = self.get_json().get_child_int('**/loader/start_value')
            if start_value != None:
                self.get_json().get('**/plt_loader').set('start_value', '0x%x' % start_value)

        if self.get_json().get('**/fs/files') is not None or self.get_json().get_child_bool('**/runner/boot_from_flash'):
            if self.get_json().get('**/flash') is not None:
                self.get_json().get('**/flash').set('preload_file', self.get_flash_preload_file())


        system = plptree.get_config_tree_from_dict(self.get_json().get_dict())

        with open('plt_config.json', 'w') as file:
            file.write(self.get_json().dump_to_string())


        os.environ['PULP_CONFIG_FILE'] = os.path.join(os.getcwd(), 'plt_config.json')

        top = system.get('vp_class')

        if top is None:
            raise Exception("The specified configuration does not contain any"
                            " top component")

        gvsoc_config = self.system_tree.get_config('gvsoc')
        debug_mode = len(gvsoc_config.get('trace')) != 0 or len(gvsoc_config.get('event')) != 0

        trace_engine = vp.trace_engine.component(name=None, config=gvsoc_config, debug=debug_mode)

        time_engine = trace_engine.new(
            name=None,
            component='vp.time_domain',
            config=system
        )

        top_comp = time_engine.new(
            name='sys',
            component=top,
            config=system.get_config('system_tree')
        )

        trace_engine.get_port('out').bind_to(top_comp.get_port('trace'))

        trace_engine.bind()

        trace_engine.post_post_build_all()

        trace_engine.pre_start_all()

        trace_engine.start_all()

        trace_engine.post_start_all()

        trace_engine.final_bind()

        trace_engine.reset_all()

        trace_engine.load_all()

        status = time_engine.run()

        trace_engine.stop_all()

        if status == 'killed':
          print ('The top engine was not responding and was killed')
          return -1
        elif status == 'error':
          return -1
        else:
          return time_engine.run_status()


        return 0
