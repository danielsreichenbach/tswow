/*
 * This file is part of tswow (https://github.com/tswow)
 *
 * Copyright (C) 2021 tswow <https://github.com/tswow/>
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
import { finish } from "../../../data/index";
import { ipaths } from "../../../data/Settings";
import { Args } from "../../../util/Args";
import { FileChangeModule } from "../../../util/FileChanges";
import { wfs } from "../../../util/FileSystem";
import { generateBLP } from "./BLP";
import { BuildTaxiMaps } from "./BuildTaxiMaps";
import { onDirtyPNG } from "./PNG";

finish('blps', () => {
    if(!Args.hasFlag('build-blp',[process.argv])) {
        return;
    }
    const isDebug = Args.hasFlag('debug',[process.argv]);
    const blpChanges = new FileChangeModule('blps');
    let files: {[key: string]: boolean} = {}
    let totalProcessed = 0;
    let totalConverted = 0;

    if(isDebug) {
        console.log('[DATASCRIPTS] Starting BLP asset processing...');
    }

    ipaths.modules.module.all().forEach(basemod=>{
        basemod.endpoints().forEach(mod=>{
            if(!mod.assets.exists()) {
                return;
            }
            if(isDebug) {
                console.log(`[DATASCRIPTS] Processing assets in module: ${mod.get()}`);
            }
            mod.assets.iterate('RECURSE','BOTH','FULL',node=>{
                if(node.isDirectory()) {
                    if(node.toDirectory().containsFile('noconvert')) {
                        return 'ENDPOINT'
                    } else {
                        return;
                    }
                }
                if(node.basename(2).toLowerCase().get() === 'worldmap') {
                    // worldmap file
                    if(node.basename().toFile().withExtension('').toLowerCase() === node.basename(1).toLowerCase()) {
                        return;
                    }

                    // overlay file
                    if(node.basename().toLowerCase().includes('overlay')) {
                        return;
                    }

                }

                // minimap files
                if(node.basename(1).toLowerCase().get() === 'minimap') {
                    return;
                }

                if(!node.isFile()) return;
                let noext = node.toFile().abs().withExtension('')
                if(files[noext.get()]) return;
                totalProcessed++;
                onDirtyPNG(noext,blpChanges,!wfs.exists(noext.withExtension('.blp')),png=>{
                    if(isDebug) {
                        console.log(`[DATASCRIPTS]   Converting: ${png.relativeTo(mod.assets).get()} -> BLP`);
                    }
                    generateBLP(png);
                    if(!wfs.exists(png.withExtension('.blp'))) {
                        throw new Error(
                            `Failed to generate blp from ${png.abs().get()}`
                        );
                    }
                    totalConverted++;
                });
                files[noext.get()] = true;
            });
        })
    })

    if(isDebug && totalProcessed > 0) {
        console.log(`[DATASCRIPTS] BLP conversion complete: ${totalConverted} converted, ${totalProcessed} candidates`);
    }

    BuildTaxiMaps();
})