/**
 * @file fmtRunner.test.ts
 * @brief kingsejong fmt 호출 테스트 (저장소의 build/bin/kingsejong 이 있어야 한다)
 *
 * 실행: npm test
 */

import { test } from 'node:test';
import * as assert from 'node:assert/strict';
import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import { findKingsejongExecutable, formatWithCli, mergeConfig } from '../fmtRunner';

// out/test 에서 저장소 루트까지 올라가면 vscode-extension 이다
const extensionPath = path.join(__dirname, '..', '..');
const exe = findKingsejongExecutable(extensionPath) ?? '';

test('kingsejong 실행 파일을 찾는다', () => {
    assert.ok(exe, 'build/bin/kingsejong 을 먼저 빌드하세요');
});

test('설정의 들여쓰기 크기가 결과에 반영된다', async () => {
    const code = '만약 참 {\n출력(1)\n}\n';
    assert.equal(await formatWithCli(exe, code, { indentSize: 2 }), '만약 참 {\n  출력(1)\n}\n');
    assert.equal(await formatWithCli(exe, code, { indentSize: 4 }), '만약 참 {\n    출력(1)\n}\n');
});

test('경로에 공백이 있어도 포맷한다', async () => {
    const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'ksj dir with space-'));
    const spaced = path.join(dir, 'kingsejong' + (process.platform === 'win32' ? '.exe' : ''));
    fs.copyFileSync(exe, spaced);
    fs.chmodSync(spaced, 0o755);
    try {
        assert.equal(await formatWithCli(spaced, 'x=1\n', {}), 'x = 1\n');
    } finally {
        fs.rmSync(dir, { recursive: true, force: true });
    }
});

test('포맷할 수 없는 코드는 오류 메시지를 담아 실패한다', async () => {
    await assert.rejects(formatWithCli(exe, '출력(\n', {}), /파싱 에러/);
});

test('프로젝트의 .ksjfmt.json 이 편집기 설정보다 우선한다', () => {
    const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'ksj-project-'));
    try {
        assert.deepEqual(mergeConfig({ indentSize: 4, useSpaces: true }, dir), { indentSize: 4, useSpaces: true });
        fs.writeFileSync(path.join(dir, '.ksjfmt.json'), JSON.stringify({ indentSize: 2 }));
        assert.deepEqual(mergeConfig({ indentSize: 4, useSpaces: true }, dir), { indentSize: 2, useSpaces: true });
        assert.deepEqual(mergeConfig({ indentSize: 4 }, undefined), { indentSize: 4 });
    } finally {
        fs.rmSync(dir, { recursive: true, force: true });
    }
});
