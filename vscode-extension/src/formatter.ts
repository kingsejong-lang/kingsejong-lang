/**
 * @file formatter.ts
 * @brief KingSejong 코드 포맷터
 * @author KingSejong Team
 * @date 2025-11-23
 *
 * 편집기 설정과 프로젝트의 .ksjfmt.json 을 합쳐 kingsejong fmt 명령어로 코드를 포맷팅합니다.
 */

import * as vscode from 'vscode';
import { FormatterConfig, formatWithCli, mergeConfig } from './fmtRunner';

/**
 * KingSejong 문서 포맷팅 제공자
 */
export class KingSejongFormattingProvider implements vscode.DocumentFormattingEditProvider {
    private formatterPath: string;

    constructor(formatterPath: string) {
        this.formatterPath = formatterPath;
    }

    /**
     * 문서 포맷팅 실행
     */
    async provideDocumentFormattingEdits(
        document: vscode.TextDocument,
        _options: vscode.FormattingOptions,
        _token: vscode.CancellationToken
    ): Promise<vscode.TextEdit[]> {
        const settings = vscode.workspace.getConfiguration('kingsejong.formatter', document.uri);
        if (!settings.get<boolean>('enable', true)) {
            return [];
        }

        try {
            const editorConfig: FormatterConfig = {
                indentSize: settings.get<number>('indentSize', 4),
                useSpaces: settings.get<boolean>('useSpaces', true),
                maxLineLength: settings.get<number>('maxLineLength', 100),
            };
            const workspaceFolder = vscode.workspace.getWorkspaceFolder(document.uri);
            const config = mergeConfig(editorConfig, workspaceFolder?.uri.fsPath);

            const text = document.getText();
            const formatted = await formatWithCli(this.formatterPath, text, config);

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
            const message = error instanceof Error ? error.message : String(error);
            vscode.window.showErrorMessage(`포맷팅 실패: ${message}`);
            return [];
        }
    }
}
