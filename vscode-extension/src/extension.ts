/**
 * @file extension.ts
 * @brief KingSejong VS Code Extension 진입점
 * @author KingSejong Team
 * @date 2025-11-12
 *
 * LSP 클라이언트를 시작하고 KingSejong 언어 서버와 통신합니다.
 */

import { workspace, ExtensionContext, languages, commands, window } from 'vscode';
import {
    DidChangeConfigurationNotification,
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind
} from 'vscode-languageclient/node';
import { KingSejongFormattingProvider } from './formatter';
import { findKingsejongExecutable } from './fmtRunner';

let client: LanguageClient;

/**
 * 언어 서버가 읽는 린터 설정 ({ lint: { enable } }).
 * initializationOptions 와 workspace/didChangeConfiguration 의 settings.kingsejong 이 같은 모양이다.
 */
function lintSettings(): { lint: { enable: boolean } } {
    return { lint: { enable: workspace.getConfiguration('kingsejong').get<boolean>('lint.enable', true) } };
}

/**
 * Extension 활성화 시 호출됩니다.
 * LSP 클라이언트를 시작하고 서버에 연결합니다.
 */
export function activate(context: ExtensionContext) {
    console.log('KingSejong extension is now active!');

    // 명령은 실행 파일이 없어도 등록해 둔다 (없으면 "명령을 찾을 수 없음" 대신 안내가 뜬다)
    context.subscriptions.push(
        commands.registerCommand('kingsejong.formatDocument', () =>
            commands.executeCommand('editor.action.formatDocument'))
    );

    // 서버 실행 파일 경로
    // 빌드된 kingsejong 실행 파일을 사용 (언어 서버와 포맷터가 같은 실행 파일이다)
    const serverModule = findKingsejongExecutable(context.extensionPath);

    if (!serverModule) {
        window.showWarningMessage(
            'kingsejong 실행 파일을 찾을 수 없어 언어 서버와 포맷터를 켜지 못했습니다. ' +
            'kingsejong 을 빌드해 PATH 에 두세요.');
        return;
    }

    // 서버 옵션 설정
    const serverOptions: ServerOptions = {
        command: serverModule,
        args: ['--lsp'],  // LSP 모드로 실행
        transport: TransportKind.stdio
    };

    // 클라이언트 옵션 설정
    const clientOptions: LanguageClientOptions = {
        // LSP를 적용할 언어 (KingSejong)
        documentSelector: [{ scheme: 'file', language: 'kingsejong' }],

        // 서버가 시작할 때 읽는 설정 (린터 진단 켜기/끄기)
        initializationOptions: lintSettings(),

        // 작업 공간 설정
        synchronize: {
            // workspace에서 .ksj 파일과 린터 설정(.ksjlint.json) 변경 감지.
            // 설정이 바뀌면 서버가 열린 문서를 다시 진단한다
            fileEvents: [
                workspace.createFileSystemWatcher('**/*.ksj'),
                workspace.createFileSystemWatcher('**/.ksjlint.json')
            ]
        }
    };

    // LSP 클라이언트 생성 및 시작
    // id 가 설정 이름의 앞부분이 된다: kingsejong.trace.server
    client = new LanguageClient(
        'kingsejong',
        'KingSejong Language Server',
        serverOptions,
        clientOptions
    );

    // 클라이언트 시작
    client.start();

    // kingsejong.lint.enable 이 바뀌면 서버에 알려 열린 문서의 진단을 다시 받는다 (재시작 불필요)
    context.subscriptions.push(workspace.onDidChangeConfiguration(event => {
        if (event.affectsConfiguration('kingsejong.lint.enable')) {
            void client.sendNotification(DidChangeConfigurationNotification.type, {
                settings: { kingsejong: lintSettings() }
            });
        }
    }));

    console.log('KingSejong LSP client started');

    // Formatter 등록
    const formattingProvider = new KingSejongFormattingProvider(serverModule);
    context.subscriptions.push(languages.registerDocumentFormattingEditProvider(
        { language: 'kingsejong', scheme: 'file' },
        formattingProvider
    ));
    console.log('KingSejong formatter registered');
}

/**
 * Extension 비활성화 시 호출됩니다.
 * LSP 클라이언트를 종료합니다.
 */
export function deactivate(): Thenable<void> | undefined {
    if (!client) {
        return undefined;
    }
    return client.stop();
}
