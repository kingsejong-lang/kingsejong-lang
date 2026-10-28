import { defineConfig } from 'vite';

// monaco-editor 의 진입점(esm/vs/index.js)은 언어 80여 개와 언어 워커(ts·css·html·json),
// LSP 클라이언트를 모두 가져온다. 플레이그라운드는 일반 텍스트만 편집하므로 그 줄을 지우고
// 편집기 본체와 기능(찾기·괄호 맞춤·명령 팔레트 등)만 남긴다.
function monacoWithoutLanguages() {
  const unused = /languages\/|__src_languages_|monaco-lsp-client|\bas lsp\b/;
  return {
    name: 'monaco-without-languages',
    transform(code, id) {
      if (!id.replace(/\\/g, '/').endsWith('/monaco-editor/esm/vs/index.js')) return null;
      return code.split('\n').filter((line) => !unused.test(line)).join('\n');
    },
  };
}

export default defineConfig({
  base: './',
  plugins: [monacoWithoutLanguages()],
  build: {
    outDir: 'dist',
    assetsDir: 'assets',
  },
  server: {
    port: 3000,
    open: true,
  },
  // WASM 파일 로딩을 위한 설정
  optimizeDeps: {
    exclude: ['kingsejong.wasm'],
  },
});
