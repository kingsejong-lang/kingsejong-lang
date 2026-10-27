/**
 * @file fmtRunner.ts
 * @brief `kingsejong fmt` 실행과 실행 파일 탐색 (vscode API 없이 테스트할 수 있게 분리)
 */

import * as path from 'path';
import * as fs from 'fs';
import * as os from 'os';
import { execFile, execFileSync } from 'child_process';

/**
 * `kingsejong fmt --config` 가 읽는 설정 (.ksjfmt.json 과 같은 키)
 */
export interface FormatterConfig {
    indentSize?: number;
    useSpaces?: boolean;
    maxLineLength?: number;
    spaceAroundOperators?: boolean;
    spaceAfterComma?: boolean;
    spaceBeforeBrace?: boolean;
}

/** 프로젝트 설정 파일 이름. CLI 가 현재 디렉터리에서 찾는 이름과 같다 */
export const PROJECT_CONFIG_FILE = '.ksjfmt.json';

/**
 * 편집기 설정 위에 프로젝트의 .ksjfmt.json 을 덮어 최종 설정을 만든다.
 * 저장소에 들어 있는 프로젝트 설정이 개인 편집기 설정보다 우선한다.
 */
export function mergeConfig(editor: FormatterConfig, projectDir: string | undefined): FormatterConfig {
    if (!projectDir) {
        return { ...editor };
    }
    const configPath = path.join(projectDir, PROJECT_CONFIG_FILE);
    if (!fs.existsSync(configPath)) {
        return { ...editor };
    }
    const project = JSON.parse(fs.readFileSync(configPath, 'utf-8')) as FormatterConfig;
    return { ...editor, ...project };
}

/**
 * 코드를 임시 파일에 써서 `kingsejong fmt --config <설정> <파일>` 로 포맷하고 결과를 돌려준다.
 * 포매터가 실패하면(구문 오류, 지원하지 않는 구문, 검증 실패) stderr 를 담아 예외를 던진다.
 */
export async function formatWithCli(executable: string, code: string, config: FormatterConfig): Promise<string> {
    // 확장 설치 폴더는 쓰기 금지일 수 있으므로 OS 임시 폴더를 쓴다
    const dir = await fs.promises.mkdtemp(path.join(os.tmpdir(), 'kingsejong-fmt-'));
    const file = path.join(dir, 'input.ksj');
    const configFile = path.join(dir, 'config.json');
    try {
        await fs.promises.writeFile(file, code, 'utf-8');
        await fs.promises.writeFile(configFile, JSON.stringify(config), 'utf-8');
        await new Promise<void>((resolve, reject) => {
            // execFile 은 셸을 거치지 않아 경로에 공백·따옴표가 있어도 안전하다
            execFile(executable, ['fmt', '--config', configFile, file], { timeout: 10000 }, (error, _stdout, stderr) => {
                if (error) {
                    reject(new Error(stderr.trim() || error.message));
                } else {
                    resolve();
                }
            });
        });
        return await fs.promises.readFile(file, 'utf-8');
    } finally {
        await fs.promises.rm(dir, { recursive: true, force: true });
    }
}

/**
 * kingsejong 실행 파일을 찾는다.
 *
 * 우선순위:
 * 1. Extension 번들에 포함된 실행 파일 (server/kingsejong)
 * 2. 개발 환경의 빌드 디렉토리 (../build/bin/kingsejong)
 * 3. 시스템 PATH
 */
export function findKingsejongExecutable(extensionPath: string): string | null {
    const exe = process.platform === 'win32' ? 'kingsejong.exe' : 'kingsejong';
    const candidates = [
        path.join(extensionPath, 'server', exe),
        path.join(extensionPath, '..', 'build', 'bin', exe),
    ];
    for (const candidate of candidates) {
        if (fs.existsSync(candidate)) {
            return candidate;
        }
    }

    const whichCommand = process.platform === 'win32' ? 'where' : 'which';
    try {
        const found = execFileSync(whichCommand, ['kingsejong'], { encoding: 'utf8' }).split(/\r?\n/)[0].trim();
        if (found) {
            return found;
        }
    } catch {
        // PATH 에 없음
    }
    return null;
}
