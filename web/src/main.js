/**
 * KingSejong 플레이그라운드 메인 스크립트
 */

import * as monaco from 'monaco-editor';
import { examples } from './examples.js';

// ============================================================================
// 전역 상태
// ============================================================================

let interpreter = null;
let editor = null;

// UI 요소
const elements = {
    status: document.getElementById('status'),
    editor: document.getElementById('editor'),
    output: document.getElementById('output'),
    runBtn: document.getElementById('runBtn'),
    clearBtn: document.getElementById('clearBtn'),
    resetBtn: document.getElementById('resetBtn'),
    examplesBtn: document.getElementById('examplesBtn'),
    examplesModal: document.getElementById('examplesModal'),
    closeModal: document.getElementById('closeModal'),
    examplesList: document.getElementById('examplesList'),
};

// ============================================================================
// 초기화
// ============================================================================

/**
 * 애플리케이션 초기화
 */
async function initialize() {
    initializeEditor();
    await loadWasmModule();
    setupEventListeners();
    renderExamples();
}

/**
 * Monaco Editor 초기화
 */
function initializeEditor() {
    // Monaco Editor 생성
    editor = monaco.editor.create(elements.editor, {
        value: examples[0].code,  // 첫 번째 예제로 초기화
        language: 'text',  // 추후 KingSejong 언어 정의 추가 가능
        theme: 'vs-dark',
        automaticLayout: true,
        fontSize: 14,
        minimap: { enabled: false },
        scrollBeyondLastLine: false,
        lineNumbers: 'on',
        renderWhitespace: 'selection',
        tabSize: 4,
    });

    // 단축키 설정
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter, () => {
        runCode();
    });

    console.log('✓ Monaco Editor initialized');
}

/**
 * WebAssembly 모듈 로드
 */
async function loadWasmModule() {
    updateStatus('loading', '⏳ WebAssembly 모듈 로딩 중...');

    try {
        // WASM 모듈 로드 (public 디렉토리에서)
        const createKingSejong = await import('/kingsejong.js');
        const module = await createKingSejong.default();

        // 인터프리터 생성
        interpreter = new module.KingSejongInterpreter();

        // 버전 확인
        const version = interpreter.version();
        updateStatus('ready', `✅ ${version}`);

        // 버튼 활성화
        elements.runBtn.disabled = false;
        elements.clearBtn.disabled = false;
        elements.resetBtn.disabled = false;
        elements.examplesBtn.disabled = false;

        console.log('✓ WASM module loaded:', version);

    } catch (error) {
        updateStatus('error', `❌ 모듈 로드 실패: ${error.message}`);
        console.error('Failed to load WASM module:', error);

        // 에러 메시지 표시
        appendOutput(
            `WebAssembly 모듈을 로드할 수 없습니다.\n\n` +
            `오류: ${error.message}\n\n` +
            `WASM 파일이 public 디렉토리에 있는지 확인하세요:\n` +
            `- public/kingsejong.js\n` +
            `- public/kingsejong.wasm`,
            'error'
        );
    }
}

// ============================================================================
// UI 업데이트
// ============================================================================

/**
 * 상태 표시 업데이트
 */
function updateStatus(type, text) {
    const statusText = elements.status.querySelector('.status-text');
    const statusIcon = elements.status.querySelector('.status-icon');

    if (statusText) statusText.textContent = text;
    if (statusIcon) statusIcon.textContent = type === 'loading' ? '⏳' : type === 'ready' ? '✅' : '❌';

    elements.status.className = 'status ' + type;
}

/**
 * 출력 영역에 텍스트 추가
 */
function appendOutput(text, type = 'normal') {
    // 플레이스홀더 제거
    const placeholder = elements.output.querySelector('.output-placeholder');
    if (placeholder) {
        placeholder.remove();
    }

    const line = document.createElement('div');
    line.className = 'output-line';

    if (type === 'error') {
        line.classList.add('output-error');
    } else if (type === 'success') {
        line.classList.add('output-success');
    } else if (type === 'info') {
        line.classList.add('output-info');
    } else if (type === 'warning') {
        line.classList.add('output-warning');
    }

    line.textContent = text;
    elements.output.appendChild(line);

    // 스크롤을 아래로
    elements.output.scrollTop = elements.output.scrollHeight;
}

/**
 * 출력 영역 초기화
 */
function clearOutput() {
    elements.output.innerHTML = '<div class="output-placeholder">출력이 지워졌습니다.</div>';
}

// ============================================================================
// 코드 실행
// ============================================================================

/**
 * 코드 실행
 */
function runCode() {
    if (!interpreter) {
        appendOutput('인터프리터가 초기화되지 않았습니다.', 'error');
        return;
    }

    const code = editor.getValue();
    if (!code.trim()) {
        appendOutput('코드를 입력해주세요.', 'info');
        return;
    }

    try {
        // 구분선 추가 (여러 번 실행 시)
        const lines = elements.output.querySelectorAll('.output-line');
        if (lines.length > 0) {
            appendOutput('─'.repeat(50), 'info');
        }

        // 코드 실행
        const result = interpreter.evaluate(code);

        if (result.startsWith('ERROR: ')) {
            // 에러
            appendOutput(result, 'error');
        } else if (result === '') {
            // 반환값 없음
            appendOutput('(실행 완료 - 반환값 없음)', 'info');
        } else {
            // 성공
            appendOutput(result, 'success');
        }

    } catch (error) {
        appendOutput(`실행 오류: ${error.message}`, 'error');
        console.error('Runtime error:', error);
    }
}

/**
 * 인터프리터 초기화
 */
function resetInterpreter() {
    if (!interpreter) return;

    interpreter.reset();
    appendOutput('✨ 환경이 초기화되었습니다.', 'info');
}

// ============================================================================
// 예제 관리
// ============================================================================

/**
 * 예제 목록 렌더링
 */
function renderExamples() {
    elements.examplesList.innerHTML = '';

    examples.forEach((example, index) => {
        const item = document.createElement('div');
        item.className = 'example-item';
        item.innerHTML = `
            <div class="example-title">${index + 1}. ${example.title}</div>
            <div class="example-description">${example.description}</div>
            <pre class="example-preview">${example.code.substring(0, 100)}${example.code.length > 100 ? '...' : ''}</pre>
        `;

        item.addEventListener('click', () => {
            loadExample(example);
        });

        elements.examplesList.appendChild(item);
    });
}

/**
 * 예제 로드
 */
function loadExample(example) {
    editor.setValue(example.code);
    closeExamplesModal();

    // 자동 실행 (선택적)
    // setTimeout(() => runCode(), 100);
}

/**
 * 예제 모달 열기
 */
function openExamplesModal() {
    elements.examplesModal.classList.add('active');
}

/**
 * 예제 모달 닫기
 */
function closeExamplesModal() {
    elements.examplesModal.classList.remove('active');
}

// ============================================================================
// 이벤트 리스너
// ============================================================================

/**
 * 이벤트 리스너 설정
 */
function setupEventListeners() {
    // 버튼 이벤트
    elements.runBtn.addEventListener('click', runCode);
    elements.clearBtn.addEventListener('click', clearOutput);
    elements.resetBtn.addEventListener('click', resetInterpreter);
    elements.examplesBtn.addEventListener('click', openExamplesModal);
    elements.closeModal.addEventListener('click', closeExamplesModal);

    // 모달 배경 클릭 시 닫기
    elements.examplesModal.addEventListener('click', (e) => {
        if (e.target === elements.examplesModal) {
            closeExamplesModal();
        }
    });

    // ESC 키로 모달 닫기
    document.addEventListener('keydown', (e) => {
        if (e.key === 'Escape') {
            closeExamplesModal();
        }
    });

    console.log('✓ Event listeners set up');
}

// ============================================================================
// 앱 시작
// ============================================================================

initialize().catch(error => {
    console.error('Initialization failed:', error);
});
