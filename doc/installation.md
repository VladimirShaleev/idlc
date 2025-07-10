@page installation Installation

@tableofcontents

# Installing and Using the Library

This section covers how to integrate the built-in compiler and use the API.

## JavaScript

### Install the NPM Package

@note I haven’t published the package to npmjs. Instead, it’s hosted in a public **GitHub** repository. You’ll need to:
* Add the **GitHub** repository to your `.npmrc` under the `NAMESPACE` `vladimirshaleev` ([GitHub repo setup](https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-npm-registry#installing-a-package)).
* Authenticate with **GitHub** to access GitHub repositories ([GitHub auth](https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-npm-registry#authenticating-to-github-packages)).

- Create an empty project (e.g., in a folder like `test-js`). Inside it, generate a package.json by running `npm init`.
- Then, add the **idlc** npm package as a dependency:
  ```
  npm install @vladimirshaleev/idlc
  ```
  <details>
  <summary>After this, your `package.json` might look like this:</summary>
  
  ```javascript
  {
    "name": "test-js",
    "main": "index.js",
    "type": "module",
    "description": "",
    "dependencies": {
      "@vladimirshaleev/idlc": "^1.5.12"
    }
  }
  ```
</details>

### Using the Embedded Compiler

Now, in `index.js`, you can add the following code:

<div class="section_buttons">

| Previous                              |                         Next |
|:--------------------------------------|-----------------------------:|
| [Language Guide](language-guide.html) | [Documentation](topics.html) |

</div>
