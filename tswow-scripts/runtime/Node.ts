import { ipaths } from "../util/Paths";
import { isWindows } from "../util/Platform";

// Wrap a path in double quotes if it contains a space. Bundled node lives
// under bin/, but on Windows installations TSWoW itself may sit under
// "C:\Program Files\..." or "C:\Users\Some Name\..." and the unquoted path
// breaks shell invocation.
function quotePath(path: string): string {
    return path.includes(' ') ? `"${path}"` : path;
}

export const NodeExecutable = isWindows() ? quotePath(ipaths.bin.node.node_exe.abs().get()) : 'node'
export const NpxExecutable = isWindows() ? quotePath(ipaths.bin.node.npx_exe.abs().get()) : 'npx'
export const NpmExecutable = isWindows() ? quotePath(ipaths.bin.node.npm_exe.abs().get()) : 'npm'
