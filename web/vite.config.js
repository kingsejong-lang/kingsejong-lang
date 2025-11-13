import { defineConfig } from 'vite';

export default defineConfig({
  base: './',
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
