
class DoxygenHighlight {
    static init() {
        document.addEventListener('DOMContentLoaded', () => {
            hljs.registerLanguage('cmake-ext', (hljs) => {
                const cmakeLanguage = hljs.getLanguage('cmake');
                return {
                    name: "CMakeExt",
                    aliases: cmakeLanguage.aliases,
                    case_insensitive: cmakeLanguage.case_insensitive,
                    contains: cmakeLanguage.contains,
                    keywords: {
                        keyword: cmakeLanguage.keywords.keyword + ' ' + 'idlc_compile'
                    }
                };
            });
            hljs.registerLanguage('idl', (hljs) => {
                const KEYWORDS = {
                    keyword: 'api enum const struct field interface method arg prop event handle func callback import'
                }
                const COMMENT = {
                    className: 'meta',
                    begin: '//',
                    end: '(?=\\n|$)'
                }
                const DOCUMENTATION = {
                    className: 'comment',
                    begin: '@\\s*(\\w+)?',
                    end: '(?=\\n|$)',
                    contains: [
                        {
                            className: 'attribute',
                            begin: '\\{',
                            end: '\\}'
                        },
                        {
                            className: 'symbol',
                            begin: '\\[',
                            end: '\\]'
                        }
                    ]
                };
                const MDOCUMENTATION = {
                    className: 'comment',
                    begin: '@\\s*```',
                    end: '```',
                    contains: [
                        {
                            className: 'attribute',
                            begin: '\\{',
                            end: '\\}'
                        },
                        {
                            className: 'symbol',
                            begin: '\\[',
                            end: '\\]'
                        }
                    ]
                };
                const LITERAL = {
                    className: 'type',
                    begin: ':\\s*\\d+',
                    end: '(?=\\n|$|@|\\[)'
                };
                const LITERAL_REF = {
                    className: 'attribute',
                    begin: ':\\s*\\w+',
                    end: '(?=\\n|$|@|\\[)'
                };
                const TYPE = {
                    className: 'attribute',
                    begin: '\\{',
                    end: '\\}'
                };
                const ATTRIBUTE = {
                    className: 'symbol',
                    begin: '\\[',
                    end: '\\]',
                    contains: [
                        {
                            className: 'type',
                            begin: '\\(\\s*\\d+',
                            end: '\\)'
                        },
                        {
                            className: 'attribute',
                            begin: '\\(\\s*\\w+',
                            end: '\\)'
                        }
                    ]
                };
                return {
                    name: "IDL",
                    case_insensitive: false,
                    keywords: KEYWORDS,
                    contains: [
                        COMMENT,
                        MDOCUMENTATION,
                        DOCUMENTATION,
                        TYPE,
                        ATTRIBUTE,
                        LITERAL,
                        LITERAL_REF
                    ]
                };
            });
        });
        $(function () {
            $(document).ready(function () {
                if (navigator.clipboard) {
                    const elements = document.getElementsByClassName("hljs")
                    for (const el of elements) {
                        DoxygenHighlight.addCopyButton(el);
                    }
                }
            })
        })
    }
    static addCopyButton(el) {
        const fragmentWrapper = document.createElement("div")
        fragmentWrapper.className = "doxygen-awesome-fragment-wrapper"
        const fragmentCopyButton = document.createElement("doxygen-awesome-fragment-copy-button")
        fragmentCopyButton.innerHTML = DoxygenAwesomeFragmentCopyButton.copyIcon
        fragmentCopyButton.title = DoxygenAwesomeFragmentCopyButton.title

        el.parentNode.replaceChild(fragmentWrapper, el)
        fragmentWrapper.appendChild(el)
        fragmentWrapper.appendChild(fragmentCopyButton)
        return fragmentCopyButton
    }
    static apply() {
        document.addEventListener('DOMContentLoaded', () => {
            document.querySelectorAll('.fragment').forEach(fragment => {
                const pre = document.createElement('pre');
                const code = document.createElement('code');

                code.className = 'language-plaintext';
                code.style.whiteSpace = 'pre';

                let skip = false;
                let first = true;
                let codeContent = '';
                fragment.querySelectorAll('.line').forEach(line => {
                    if (first) {
                        const lang = line.textContent;
                        if (lang == '@cmake') {
                            code.className = `language-cmake-ext`;
                        } else if (lang == '@idl') {
                            code.className = `language-idl`;
                        } else if (lang == '@json') {
                            code.className = `language-json`;
                        } else if (lang == '@c') {
                            code.className = `language-c`;
                        } else if (lang == '@cpp') {
                            code.className = `language-cpp`;
                        } else if (lang == '@javascript') {
                            code.className = `language-javascript`;
                        } else if (lang == '@bash') {
                            code.className = `language-bash`;
                        } else {
                            skip = true;
                        }
                    } else {
                        codeContent += line.textContent + '\n';
                    }
                    first = false;
                });

                if (!skip) {
                    code.textContent = codeContent.trimEnd();
                    pre.appendChild(code);
                    fragment.replaceWith(pre);
                }
            });

            if (typeof hljs !== 'undefined') {
                hljs.highlightAll();
            }
        });
    }
}
