#!/usr/bin/env node

/**
 * Hamlib build script
 * Builds the latest version of Hamlib from GitHub source
 *
 * Usage:
 *   node scripts/build-hamlib.js [options]
 *
 * Options:
  *   --prefix=<path>   Custom installation path (default: ./hamlib-build for CI, /usr/local for manual)
 *   --minimal         Minimal build, reduces dependencies and build time
 *   --help            Show help information
 */

const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');
const os = require('os');
const { createLogger } = require('./logger');

const logger = createLogger('Building Hamlib');

function exec(command, options = {}) {
  try {
    return execSync(command, {
      stdio: options.silent ? 'pipe' : (logger.verbose ? 'inherit' : 'pipe'),
      encoding: 'utf-8',
      shell: true,
      ...options
    });
  } catch (error) {
    if (options.ignoreError) {
      return null;
    }
    throw error;
  }
}

// Parse command line arguments
function parseArgs() {
  const args = process.argv.slice(2);
  const config = {
    prefix: null,
    minimal: false,
    help: false,
  };

  for (const arg of args) {
    if (arg.startsWith('--prefix=')) {
      config.prefix = arg.substring(9);
    } else if (arg === '--minimal') {
      config.minimal = true;
    } else if (arg === '--help' || arg === '-h') {
      config.help = true;
    } else {
      logger.warning(`Unknown argument: ${arg}`);
    }
  }

  return config;
}

function showHelp() {
  console.log(`
Hamlib Build Script

Usage:
  node scripts/build-hamlib.js [options]

Options:
  --prefix=<path>   Custom installation path
                    Default: ./hamlib-build

  --minimal         Minimal build, disables unnecessary features
                    Reduces dependencies and build time

  --help, -h        Show this help information

Examples:
  # CI environment (auto-detected)
  node scripts/build-hamlib.js --minimal

  # Install to project-local directory (no sudo required)
  node scripts/build-hamlib.js --prefix=$HOME/local/hamlib

  # Install to system path (requires sudo)
  node scripts/build-hamlib.js --prefix=/usr/local

Notes:
  - Linux/macOS: Requires autoconf, automake, libtool
  - Windows: Script will skip, use official prebuilt packages
  - Build time: ~5-10 minutes (depends on CPU and configuration)
`);
}

// Detect platform
function detectPlatform() {
  const platform = os.platform();
  if (platform === 'win32') {
    return 'windows';
  } else if (platform === 'darwin') {
    return 'macos';
  } else if (platform === 'linux') {
    return 'linux';
  } else {
    return 'unknown';
  }
}

// Detect CI environment
function isCI() {
  return !!(process.env.CI || process.env.GITHUB_ACTIONS);
}

// Check required build tools
function checkBuildTools(platform) {
  // macOS uses glibtoolize, Linux uses libtoolize
  const libtoolCmd = platform === 'macos' ? 'glibtoolize' : 'libtoolize';
  const requiredTools = ['git', 'autoconf', 'automake', libtoolCmd, 'make'];
  const missingTools = [];

  for (const tool of requiredTools) {
    try {
      exec(`${tool} --version 2>&1`, { silent: true, ignoreError: false });
      logger.log(`Found ${tool}`, 'success');
    } catch (error) {
      missingTools.push(tool);
    }
  }

  if (missingTools.length > 0) {
    const installCmd = platform === 'macos'
      ? 'brew install autoconf automake libtool'
      : 'sudo apt-get install autoconf automake libtool pkg-config';
    throw new Error(`Missing required tools: ${missingTools.join(', ')}\nInstall: ${installCmd}`);
  }

  logger.log('All required tools installed', 'success');
}

// Clone Hamlib repository
function cloneHamlib(workDir) {
  const hamlibDir = path.join(workDir, 'Hamlib');

  if (fs.existsSync(hamlibDir)) {
    logger.log('Hamlib directory exists, removing...', 'warning');
    exec(`rm -rf "${hamlibDir}"`);
  }

  logger.startSpinner('Cloning Hamlib from GitHub...');
  const hamlibVersion = process.env.HAMLIB_BRANCH || '4.7.1';
  const hamlibRepoUrl = process.env.HAMLIB_REPO || 'https://github.com/Hamlib/Hamlib.git';
  exec(`git clone --depth 1 --branch ${hamlibVersion} ${hamlibRepoUrl} "${hamlibDir}"`);
  logger.succeedSpinner(`Repository cloned (tag ${hamlibVersion})`);

  return hamlibDir;
}

// Build Hamlib
function buildHamlib(hamlibDir, prefix, minimal) {
  process.chdir(hamlibDir);

  // Run bootstrap
  logger.startSpinner('Running ./bootstrap...');
  exec('./bootstrap');
  logger.succeedSpinner('Bootstrap complete');

  // Configure build options
  let configureOptions = [
    `--prefix="${prefix}"`,
    '--enable-shared=yes',
    '--enable-static=no',
  ];

  if (minimal) {
    logger.log('Using minimal build configuration', 'info');
    configureOptions = configureOptions.concat([
      '--disable-winradio',
      '--without-cxx-binding',
      '--without-perl-binding',
      '--without-python-binding',
      '--without-tcl-binding',
      '--without-lua-binding',
      '--without-readline',
      '--without-xml-support',
    ]);
  }

  const configureCmd = `./configure ${configureOptions.join(' ')}`;
  logger.startSpinner('Configuring build...');
  logger.debug(`Configure: ${configureCmd}`);
  exec(configureCmd);
  logger.succeedSpinner('Configuration complete');

  // Compile
  const cores = os.cpus().length;
  logger.startSpinner(`Compiling with ${cores} CPU cores...`);
  exec(`make -j${cores}`);
  logger.succeedSpinner('Compilation complete');

  // Install
  const needsSudo = !fs.existsSync(prefix) && prefix.startsWith('/usr');
  const installCmd = needsSudo ? 'sudo make install' : 'make install';

  if (needsSudo) {
    logger.log('System path requires sudo permission', 'warning');
  }

  logger.startSpinner('Installing Hamlib...');
  exec(installCmd);
  logger.succeedSpinner('Installation complete');
}

// Verify installation
function verifyInstallation(prefix, platform) {
  const includeDir = path.join(prefix, 'include', 'hamlib');
  const libDir = path.join(prefix, 'lib');

  // Check header files
  if (fs.existsSync(includeDir)) {
    const headers = fs.readdirSync(includeDir).filter(f => f.endsWith('.h'));
    logger.log(`Found ${headers.length} header files in ${includeDir}`, 'success');
  } else {
    logger.log(`Header directory not found: ${includeDir}`, 'error');
  }

  // Check library files
  if (fs.existsSync(libDir)) {
    const libs = fs.readdirSync(libDir);

    let foundLib = false;
    if (platform === 'macos') {
      const dylibFiles = libs.filter(f => f.includes('libhamlib') && f.endsWith('.dylib'));
      if (dylibFiles.length > 0) {
        dylibFiles.forEach(f => logger.debug(`  ${f}`));
        foundLib = true;
      }
    } else if (platform === 'linux') {
      const soFiles = libs.filter(f => f.includes('libhamlib') && f.includes('.so'));
      if (soFiles.length > 0) {
        soFiles.forEach(f => logger.debug(`  ${f}`));
        foundLib = true;
      }
    }

    if (foundLib) {
      logger.log('Hamlib libraries installed', 'success');
    } else {
      logger.log('Hamlib library files not found', 'warning');
    }
  } else {
    logger.log(`Library directory not found: ${libDir}`, 'error');
  }

  // Output environment variable setup instructions
  logger.section('Environment Variables');
  logger.info('Set the following environment variables when building node-hamlib:');
  console.log(`  export HAMLIB_PREFIX="${prefix}"`);

  if (platform === 'linux') {
    console.log(`  export LD_LIBRARY_PATH="${libDir}:$LD_LIBRARY_PATH"`);
  } else if (platform === 'macos') {
    console.log(`  export DYLD_LIBRARY_PATH="${libDir}:$DYLD_LIBRARY_PATH"`);
  }

  console.log(`  export PKG_CONFIG_PATH="${path.join(libDir, 'pkgconfig')}:$PKG_CONFIG_PATH"`);
  console.log();
  console.log('Then run:');
  console.log('  npm run rebuild');
  console.log();
}

// Main function
async function main() {
  const startTime = Date.now();
  const config = parseArgs();

  if (config.help) {
    showHelp();
    process.exit(0);
  }

  logger.start();

  const platform = detectPlatform();
  const ci = isCI();

  logger.log(`Platform: ${platform}`, 'info');
  logger.log(`CI environment: ${ci ? 'Yes' : 'No'}`, 'info');

  // Skip Windows platform
  if (platform === 'windows') {
    logger.warning('Windows platform does not support source build');
    logger.info('Use official prebuilt packages: https://github.com/Hamlib/Hamlib/releases');
    process.exit(0);
  }

  // Determine installation path
  let prefix = config.prefix;
  if (!prefix) {
    if (ci) {
      prefix = path.resolve(process.cwd(), 'hamlib-build');
      logger.log(`CI environment, using local path: ${prefix}`, 'info');
    } else {
      prefix = path.resolve(process.cwd(), 'hamlib-build');
      logger.log(`Default installation path: ${prefix}`, 'info');
    }
  }

  logger.log(`Installation path: ${prefix}`, 'success');

  try {
    // Check build tools
    logger.step('Checking build tools', 1, 5);
    checkBuildTools(platform);

    // Create work directory
    const workDir = path.join(os.tmpdir(), `hamlib-build-${Date.now()}`);
    fs.mkdirSync(workDir, { recursive: true });
    logger.debug(`Work directory: ${workDir}`);

    // Clone repository
    logger.step('Cloning Hamlib repository', 2, 5);
    const hamlibDir = cloneHamlib(workDir);

    // Build
    logger.step('Building Hamlib', 3, 5);
    buildHamlib(hamlibDir, prefix, config.minimal);

    // Verify
    logger.step('Verifying installation', 4, 5);
    verifyInstallation(prefix, platform);

    // Cleanup
    logger.step('Cleaning up', 5, 5);
    exec(`rm -rf "${workDir}"`, { ignoreError: true });
    logger.log('Temporary files removed', 'success');

    const duration = (Date.now() - startTime) / 1000;
    logger.success(duration);

  } catch (error) {
    logger.error('Build failed', error.message, [
      'Check that all build tools are installed',
      'Ensure network connection for git clone',
      'Verify installation path permissions'
    ]);
    process.exit(1);
  }
}

// Run main function
main().catch(error => {
  console.error('Unexpected error:', error.message);
  process.exit(1);
});
