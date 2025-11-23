/**
 * @file formatter.ts
 * @brief KingSejong 코드 포맷터
 * @author KingSejong Team
 * @date 2025-11-23
 *
 * .ksjfmtrc 파일을 읽어 kingsejong fmt 명령어를 실행하여 코드를 포맷팅합니다.
 */

import * as vscode from 'vscode';
import * as path from 'path';
import * as fs from 'fs';
import { exec } from 'child_process';
import { promisify } from 'util';

const execAsync = promisify(exec);

/**
 * .ksjfmtrc 포맷 설정
 */
interface FormatterConfig {
    indentSize?: number;
    useSpaces?: boolean;
    maxLineLength?: number;
    insertFinalNewline?: boolean;
    trimTrailingWhitespace?: boolean;
}

/**
 * KingSejong 문서 포맷팅 제공자
 */
export class KingSejongFormattingProvider implements vscode.DocumentFormattingEditProvider {
    private formatterPath: string | null = null;

    constructor(formatterPath: string | null) {
        this.formatterPath = formatterPath;
    }

    /**
     * 문서 포맷팅 실행
     */
    async provideDocumentFormattingEdits(
        document: vscode.TextDocument,
        options: vscode.FormattingOptions,
        token: vscode.CancellationToken
    ): Promise<vscode.TextEdit[]> {
        if (!this.formatterPath) {
            vscode.window.showErrorMessage('KingSejong formatter를 찾을 수 없습니다.');
            return [];
        }

        try {
            // .ksjfmtrc 파일 찾기
            const config = await this.loadFormatterConfig(document.uri);

            // 현재 문서 내용 가져오기
            const text = document.getText();

            // kingsejong fmt 명령 실행
            const formatted = await this.formatCode(text, config);

            // 변경사항이 없으면 빈 배열 반환
            if (formatted === text) {
                return [];
            }

            // 전체 문서를 포맷팅된 텍스트로 교체
            const fullRange = new vscode.Range(
                document.positionAt(0),
                document.positionAt(text.length)
            );

            return [vscode.TextEdit.replace(fullRange, formatted)];
        } catch (error) {
            vscode.window.showErrorMessage(`포맷팅 실패: ${error}`);
            return [];
        }
    }

    /**
     * .ksjfmtrc 파일을 찾아서 설정을 로드합니다.
     */
    private async loadFormatterConfig(documentUri: vscode.Uri): Promise<FormatterConfig> {
        const workspaceFolder = vscode.workspace.getWorkspaceFolder(documentUri);
        if (!workspaceFolder) {
            return this.getDefaultConfig();
        }

        const configPath = path.join(workspaceFolder.uri.fsPath, '.ksjfmtrc');

        try {
            if (fs.existsSync(configPath)) {
                const configContent = await fs.promises.readFile(configPath, 'utf-8');
                const config = JSON.parse(configContent);
                return { ...this.getDefaultConfig(), ...config };
            }
        } catch (error) {
            console.warn('Failed to load .ksjfmtrc:', error);
        }

        return this.getDefaultConfig();
    }

    /**
     * 기본 포맷터 설정
     */
    private getDefaultConfig(): FormatterConfig {
        return {
            indentSize: 4,
            useSpaces: true,
            maxLineLength: 100,
            insertFinalNewline: true,
            trimTrailingWhitespace: true
        };
    }

    /**
     * kingsejong fmt 명령어를 실행하여 코드를 포맷팅합니다.
     */
    private async formatCode(code: string, config: FormatterConfig): Promise<string> {
        if (!this.formatterPath) {
            throw new Error('Formatter path not set');
        }

        // 임시 파일 생성
        const tmpFile = path.join(__dirname, `temp-${Date.now()}.ksj`);

        try {
            // 임시 파일에 코드 쓰기
            await fs.promises.writeFile(tmpFile, code, 'utf-8');

            // config 파일 생성
            const tmpConfig = path.join(__dirname, `temp-config-${Date.now()}.json`);
            await fs.promises.writeFile(tmpConfig, JSON.stringify(config), 'utf-8');

            // kingsejong fmt 실행
            const command = `"${this.formatterPath}" fmt --config="${tmpConfig}" "${tmpFile}"`;
            const { stdout, stderr } = await execAsync(command, {
                timeout: 10000 // 10초 타임아웃
            });

            if (stderr) {
                console.warn('Formatter stderr:', stderr);
            }

            // 포맷팅된 파일 읽기
            const formatted = await fs.promises.readFile(tmpFile, 'utf-8');

            // 임시 파일 삭제
            await fs.promises.unlink(tmpFile);
            await fs.promises.unlink(tmpConfig);

            return formatted;
        } catch (error) {
            // 임시 파일 정리
            try {
                if (fs.existsSync(tmpFile)) {
                    await fs.promises.unlink(tmpFile);
                }
            } catch (e) {
                // 무시
            }

            throw error;
        }
    }
}

/**
 * 포맷터 경로를 찾습니다.
 */
export function findFormatterExecutable(extensionPath: string): string | null {
    // 1. Extension 번들에 포함된 formatter
    const bundledFormatter = path.join(extensionPath, 'server', 'kingsejong');
    if (fs.existsSync(bundledFormatter)) {
        return bundledFormatter;
    }

    // 2. 개발 환경 - 상위 디렉토리의 build/bin/kingsejong
    const devFormatter = path.join(extensionPath, '..', 'build', 'bin', 'kingsejong');
    if (fs.existsSync(devFormatter)) {
        return devFormatter;
    }

    // 3. 시스템 PATH
    const whichCommand = process.platform === 'win32' ? 'where' : 'which';
    try {
        const { execSync } = require('child_process');
        const systemFormatter = execSync(`${whichCommand} kingsejong`, { encoding: 'utf8' }).trim();
        if (systemFormatter) {
            return systemFormatter;
        }
    } catch (e) {
        // Not found in PATH
    }

    return null;
}
