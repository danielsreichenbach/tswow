import { commands } from "../util/Commands";
import { ipaths } from "../util/Paths";
import { term } from "../util/Terminal";
import { AuthServer } from "./AuthServer";
import { BuildCommand } from "./CommandActions";
import { Identifier } from "./Identifiers";
import { NodeConfig } from "./NodeConfig";
import { Realm } from "./Realm";

export class MiscCommands {
    static initialize() {
        term.debug('misc', `Initializing misc commands`)
        BuildCommand.addCommand(
              'all'
            , '(see arguments to build datscripts/addon/livescripts)'
            , 'Builds datascripts/addons/livescripts'
        , async args=>{
            let datasets = Identifier.getDatasets(args,'MATCH_ANY',NodeConfig.DefaultDataset)
            for(const dataset of datasets) {
                let runningClients = [dataset.client]
                let runningWorldservers = dataset.realms()

                await commands.sendCommand(`build data ${dataset.name} ${args} --no-restart`);
                await commands.sendCommand(`build addon ${dataset.name} ${args}`)
                // we've already built inlinescripts, skip them
                await commands.sendCommand(`build scripts ${dataset.name} ${args} --no-inline`)
                // The previous `build lua ...` invocation here pointed at a
                // command that doesn't exist; `build scripts` (alias for
                // `build livescripts`) above already runs the lua or c++
                // backend per the module's livescripts.conf.

                await Promise.all(runningClients.map(x=>x.startup(NodeConfig.AutoStartClient)))
                let autorealms = NodeConfig.AutoStartRealms
                .map(x=>Identifier.getRealm(x))
                await Promise.all(runningWorldservers
                    .filter(x=>autorealms.find(y=>y.fullName===x.fullName))
                    .map(x=>x.start(x.lastBuildType)))
            }
        })

        commands.addCommand('check','','Checks TSWoW installation and configuration',(args)=>{
            return commands.sendCommand(`build data ${args} --readonly`)
        });

        commands.addCommand('revision','','Shows TSWoW and TrinityCore revision information',()=>{
            console.log(
                  `TSWoW Revision: ${ipaths.bin.revisions.tswow.readString().slice(0,7)}\n`
                + `TrinityCore Revision: ${ipaths.bin.revisions.trinitycore.readString().slice(0,7)}`
            )
        }).addAlias('version')

        commands.addCommand('exit','','Stops all running services and exits TSWoW', async ()=>{
            term.log('misc', 'Shutting down all services...');

            const runningRealms = Realm.all().filter(x => x.worldserver.isRunning());
            if(runningRealms.length > 0) {
                term.log('misc', `Stopping ${runningRealms.length} running realm(s)...`);
                await Promise.all(runningRealms.map(x => x.worldserver.stop()));
            }

            if(AuthServer.isStarted()) {
                term.log('misc', 'Stopping authserver...');
                await AuthServer.stop();
            }

            term.success('misc', 'All services stopped. Goodbye!');
            process.exit(0);
        }).addAlias('quit')
    }
}