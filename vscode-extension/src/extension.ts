/**
 * @file extension.ts
 * @brief KingSejong VS Code Extension 진입점
 * @author KingSejong Team
 * @date 2025-11-12
 *
 * LSP 클라이언트를 시작하고 KingSejong 언어 서버와 통신합니다.
 */

import * as path from 'path';
import { workspace, ExtensionContext, languages } from 'vscode';
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind
} from 'vscode-languageclient/node';
import { KingSejongFormattingProvider, findFormatterExecutable } from './formatter';

let client: LanguageClient;

/**
 * Extension 활성화 시 호출됩니다.
 * LSP 클라이언트를 시작하고 서버에 연결합니다.
 */
export function activate(context: ExtensionContext) {
    console.log('KingSejong extension is now active!');

    // 서버 실행 파일 경로
    // 빌드된 kingsejong 실행 파일을 사용
    const serverModule = findServerExecutable(context);

    if (!serverModule) {
        console.error('KingSejong language server not found!');
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

        // 작업 공간 설정
        synchronize: {
            // workspace에서 .ksj 파일 변경 감지
            fileEvents: workspace.createFileSystemWatcher('**/*.ksj')
        }
    };

    // LSP 클라이언트 생성 및 시작
    client = new LanguageClient(
        'kingsejonglsp',
        'KingSejong Language Server',
        serverOptions,
        clientOptions
    );

    // 클라이언트 시작
    client.start();

    console.log('KingSejong LSP client started');

    // Formatter 등록
    const formatterPath = findFormatterExecutable(context.extensionPath);
    if (formatterPath) {
        const formattingProvider = new KingSejongFormattingProvider(formatterPath);
        const disposable = languages.registerDocumentFormattingEditProvider(
            { language: 'kingsejong', scheme: 'file' },
            formattingProvider
        );
        context.subscriptions.push(disposable);
        console.log('KingSejong formatter registered');
    } else {
        console.warn('KingSejong formatter not found - formatting disabled');
    }
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

/**
 * KingSejong 언어 서버 실행 파일을 찾습니다.
 *
 * 우선순위:
 * 1. Extension 번들에 포함된 서버
 * 2. 개발 환경의 빌드 디렉토리
 * 3. 시스템 PATH
 */
function findServerExecutable(context: ExtensionContext): string | null {
    const fs = require('fs');

    // 1. Extension 번들에 포함된 서버
    const bundledServer = path.join(context.extensionPath, 'server', 'kingsejong');
    if (fs.existsSync(bundledServer)) {
        console.log('Using bundled server:', bundledServer);
        return bundledServer;
    }

    // 2. 개발 환경 - 상위 디렉토리의 build/bin/kingsejong
    const devServer = path.join(context.extensionPath, '..', 'build', 'bin', 'kingsejong');
    if (fs.existsSync(devServer)) {
        console.log('Using development server:', devServer);
        return devServer;
    }

    // 3. 시스템 PATH에서 찾기
    const whichCommand = process.platform === 'win32' ? 'where' : 'which';
    try {
        const { execSync } = require('child_process');
        const systemServer = execSync(`${whichCommand} kingsejong`, { encoding: 'utf8' }).trim();
        if (systemServer) {
            console.log('Using system server:', systemServer);
            return systemServer;
        }
    } catch (e) {
        // kingsejong가 PATH에 없음
    }

    console.error('KingSejong server not found in any location');
    return null;
}
