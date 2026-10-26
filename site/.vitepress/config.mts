// KingSejong 문서 사이트 설정.
// 저장소의 docs/*.md 를 그대로 페이지로 만들고, Pages 에서는 플레이그라운드 아래 /docs/ 경로에 놓인다.
import { readFileSync } from 'node:fs'
import { createRequire } from 'node:module'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'
import { defineConfig } from 'vitepress'
import type MarkdownIt from 'markdown-it'

const REPO = 'https://github.com/kingsejong-lang/kingsejong-lang'
const vueDir = dirname(createRequire(import.meta.url).resolve('vue/package.json'))
const PLAYGROUND = 'https://kingsejong-lang.github.io/kingsejong-lang/'
const API_REFERENCE = 'https://kingsejong-lang.github.io/kingsejong-lang/api/'
const SITE = PLAYGROUND + 'docs/'
/** 공유 미리보기 이미지. web/public/og.png 가 플레이그라운드 루트에 배포된다 (scripts/generate_og_image.py 로 만든다) */
const OG_IMAGE = PLAYGROUND + 'og.png'
/** 파비콘: 세종의 첫 자음 ㅅ 을 그린 SVG. 파일 없이 data URI 로 심어 플레이그라운드와 같은 그림을 쓴다 */
const FAVICON =
  'data:image/svg+xml,' +
  encodeURIComponent(
    '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64">' +
      '<rect width="64" height="64" rx="14" fill="#3451b2"/>' +
      '<path d="M32 13 L13 52 M32 13 L51 52" stroke="#fff" stroke-width="8" stroke-linecap="round" fill="none"/>' +
      '</svg>',
  )

/** VS Code 확장의 TextMate 문법으로 ```ksj 코드 블록을 강조한다 */
const ksjGrammar = JSON.parse(
  readFileSync(
    fileURLToPath(new URL('../../vscode-extension/syntaxes/kingsejong.tmLanguage.json', import.meta.url)),
    'utf-8',
  ),
)

/**
 * 문서 밖을 가리키는 상대 링크(../examples/, ../CHANGELOG.md, ../src/...)를 GitHub 의 해당 파일로 보낸다.
 * 사이트에는 docs/ 아래 페이지만 있으므로 그대로 두면 404 가 된다. docs/ 안에 머무는 링크는 건드리지 않는다.
 */
function repoLinks(md: MarkdownIt) {
  const resolve = (pageDir: string, href: string): { path: string; isDir: boolean } | null => {
    const [target] = href.split(/[#?]/, 1)
    const out: string[] = []
    for (const seg of ('docs/' + pageDir + target).split('/')) {
      if (seg === '' || seg === '.') continue
      if (seg === '..') {
        if (out.length === 0) return null
        out.pop()
        continue
      }
      out.push(seg)
    }
    return { path: out.join('/'), isDir: target.endsWith('/') }
  }

  md.core.ruler.push('repo_links', (state) => {
    const relativePath: string = state.env?.relativePath ?? ''
    const pageDir = relativePath.includes('/') ? relativePath.slice(0, relativePath.lastIndexOf('/') + 1) : ''
    const walk = (tokens: any[]) => {
      for (const token of tokens) {
        if (token.type === 'link_open') {
          const href = token.attrGet('href')
          if (href && href.startsWith('../')) {
            const resolved = resolve(pageDir, href)
            if (resolved && !resolved.path.startsWith('docs/')) {
              token.attrSet('href', `${REPO}/${resolved.isDir ? 'tree' : 'blob'}/main/${resolved.path}`)
              token.attrSet('target', '_blank')
              token.attrSet('rel', 'noreferrer')
            }
          }
        }
        if (token.children) walk(token.children)
      }
    }
    walk(state.tokens)
  })
}

export default defineConfig({
  lang: 'ko-KR',
  title: 'KingSejong',
  description: '한글 문법으로 쓰는 프로그래밍 언어 KingSejong 의 문서',
  srcDir: '../docs',
  srcExclude: ['research/**'],
  base: '/kingsejong-lang/docs/',

  head: [
    ['link', { rel: 'icon', type: 'image/svg+xml', href: FAVICON }],
    ['meta', { property: 'og:type', content: 'website' }],
    ['meta', { property: 'og:site_name', content: 'KingSejong' }],
    ['meta', { property: 'og:locale', content: 'ko_KR' }],
    ['meta', { property: 'og:image', content: OG_IMAGE }],
    ['meta', { property: 'og:image:width', content: '1200' }],
    ['meta', { property: 'og:image:height', content: '630' }],
    ['meta', { property: 'og:image:alt', content: 'KingSejong - 한글 문법으로 쓰는 프로그래밍 언어' }],
    ['meta', { name: 'twitter:card', content: 'summary_large_image' }],
  ],

  /** 페이지마다 공유 미리보기(Open Graph)와 canonical 주소를 넣는다. 제목·설명은 각 문서의 것을 쓴다 */
  transformPageData(pageData) {
    const isHome = pageData.frontmatter.layout === 'home'
    const title = isHome || !pageData.title ? 'KingSejong 문서' : `${pageData.title} | KingSejong`
    const description =
      pageData.description || pageData.frontmatter.description || '한글 문법으로 쓰는 프로그래밍 언어 KingSejong 의 문서'
    const url = SITE + pageData.relativePath.replace(/index\.md$/, '').replace(/\.md$/, '.html')
    pageData.frontmatter.head ??= []
    pageData.frontmatter.head.push(
      ['meta', { property: 'og:title', content: title }],
      ['meta', { property: 'og:description', content: description }],
      ['meta', { property: 'og:url', content: url }],
      ['link', { rel: 'canonical', href: url }],
    )
  },

  // srcDir 이 프로젝트 루트 밖(../docs)이라, 마크다운에서 나오는 vue 가져오기를 루트의 node_modules 에서 해석하게 한다
  vite: {
    resolve: {
      dedupe: ['vue'],
      alias: [
        { find: /^vue$/, replacement: vueDir },
        { find: /^vue\/server-renderer$/, replacement: join(vueDir, 'server-renderer') },
      ],
    },
  },

  markdown: {
    languages: [{ ...ksjGrammar, name: 'ksj', aliases: ['kingsejong', 'sejong', 'ks'] }],
    config: repoLinks,
  },

  themeConfig: {
    nav: [
      { text: '튜토리얼', link: '/TUTORIAL' },
      { text: '언어 레퍼런스', link: '/LANGUAGE_REFERENCE' },
      { text: '표준 라이브러리', link: '/STDLIB_REFERENCE' },
      { text: '플레이그라운드', link: PLAYGROUND },
      { text: 'C++ API', link: API_REFERENCE },
    ],

    sidebar: [
      {
        text: '시작하기',
        items: [
          { text: '설치 가이드', link: '/INSTALL' },
          { text: '튜토리얼', link: '/TUTORIAL' },
          { text: '사용자 가이드', link: '/USER_GUIDE' },
        ],
      },
      {
        text: '레퍼런스',
        items: [
          { text: '언어 레퍼런스', link: '/LANGUAGE_REFERENCE' },
          { text: '표준 라이브러리', link: '/STDLIB_REFERENCE' },
          { text: '표준 라이브러리 확장', link: '/LIBRARY_GUIDE' },
          { text: 'Linter 규칙', link: '/LINTER_RULES' },
          { text: '설정 파일', link: '/CONFIG_GUIDE' },
          { text: '디버거', link: '/DEBUGGER_GUIDE' },
        ],
      },
      {
        text: '프로젝트',
        items: [
          { text: '로드맵', link: '/ROADMAP' },
          { text: 'v0.6 마이그레이션', link: '/MIGRATION_v0.6' },
          { text: '기여 가이드', link: '/CONTRIBUTING' },
          { text: '코딩 스타일', link: '/CODING_STYLE' },
          { text: 'C++ API 레퍼런스 (Doxygen)', link: API_REFERENCE },
          { text: '플레이그라운드 개발', link: '/PLAYGROUND_TUTORIAL' },
          { text: 'Phase 9 리팩토링 계획', link: '/PHASE9_REFACTORING_PLAN' },
        ],
      },
      {
        text: '설계 문서',
        items: [
          { text: '프로젝트 개요', link: '/design/PROJECT_OVERVIEW' },
          { text: '설계 철학', link: '/design/LANGUAGE_DESIGN_PHILOSOPHY' },
          { text: '기능 명세', link: '/design/FEATURES' },
          { text: '구현 상태', link: '/design/IMPLEMENTATION' },
          { text: '메모리 풀링 설계', link: '/design/MEMORY_POOLING_DESIGN' },
        ],
      },
    ],

    socialLinks: [{ icon: 'github', link: REPO }],

    editLink: {
      pattern: `${REPO}/edit/main/docs/:path`,
      text: 'GitHub 에서 이 문서 고치기',
    },

    outline: { level: [2, 3], label: '목차' },
    docFooter: { prev: '이전', next: '다음' },
    sidebarMenuLabel: '메뉴',
    returnToTopLabel: '맨 위로',
    darkModeSwitchLabel: '테마',
    lightModeSwitchTitle: '밝은 테마로',
    darkModeSwitchTitle: '어두운 테마로',

    search: {
      provider: 'local',
      options: {
        translations: {
          button: { buttonText: '검색', buttonAriaLabel: '검색' },
          modal: {
            noResultsText: '결과 없음',
            resetButtonTitle: '지우기',
            footer: { selectText: '선택', navigateText: '이동', closeText: '닫기' },
          },
        },
      },
    },

    footer: {
      message: 'MIT License',
      copyright: 'KingSejong Team',
    },
  },
})
