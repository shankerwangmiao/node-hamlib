import { EventEmitter } from 'events';

/**
 * Connection information interface
 */
interface ConnectionInfo {
  /** Connection type: serial or network */
  connectionType: 'serial' | 'network';
  /** Port path or network address */
  portPath: string;
  /** Connection status */
  isOpen: boolean;
  /** Original model number */
  originalModel: number;
  /** Actual model number used */
  currentModel: number;
  /** @deprecated Use isOpen */
  connected?: boolean;
  /** @deprecated Use currentModel */
  actualModel?: number;
}

/**
 * Radio mode information interface
 */
interface ModeInfo {
  /** Radio mode character */
  mode: string;
  /** Bandwidth in Hz */
  bandwidth: number;
}

/**
 * Supported rig information interface
 */
interface SupportedRigInfo {
  /** Rig model number */
  rigModel: number;
  /** Model name */
  modelName: string;
  /** Manufacturer name */
  mfgName: string;
  /** Driver version */
  version: string;
  /** Driver status (Alpha, Beta, Stable, etc.) */
  status: string;
  /** Rig type (Transceiver, Handheld, Mobile, etc.) */
  rigType: string;
}

interface SupportedRotatorInfo {
  rotModel: number;
  modelName: string;
  mfgName: string;
  version: string;
  status: string;
  rotType: 'azimuth' | 'elevation' | 'azel' | 'other';
  rotTypeMask: number;
}

/**
 * Antenna information interface
 */
interface AntennaInfo {
  /** Currently selected antenna */
  currentAntenna: number;
  /** TX antenna selection */
  txAntenna: number;
  /** RX antenna selection */
  rxAntenna: number;
  /** Additional antenna option/parameter */
  option: number;
}

interface RotatorConnectionInfo {
  connectionType: 'serial' | 'network';
  portPath: string;
  isOpen: boolean;
  originalModel: number;
  currentModel: number;
}

interface RotatorPosition {
  azimuth: number;
  elevation: number;
}

interface RotatorStatus {
  mask: number;
  flags: string[];
}

type RotatorDirection =
  | 'UP'
  | 'DOWN'
  | 'LEFT'
  | 'RIGHT'
  | 'CCW'
  | 'CW'
  | 'UP_LEFT'
  | 'UP_RIGHT'
  | 'DOWN_LEFT'
  | 'DOWN_RIGHT'
  | 'UP_CCW'
  | 'UP_CW'
  | 'DOWN_CCW'
  | 'DOWN_CW'
  | number;

type RotatorResetType = 'ALL' | number;

interface RotatorCaps {
  rotType: 'azimuth' | 'elevation' | 'azel' | 'other';
  rotTypeMask: number;
  minAz: number;
  maxAz: number;
  minEl: number;
  maxEl: number;
  supportedStatuses: string[];
}

/**
 * Hamlib VFO token.
 * 实际支持的 token 取决于具体 backend / 电台型号。
 */
type VFO =
  | 'VFOA'
  | 'VFOB'
  | 'VFOC'
  | 'currVFO'
  | 'VFO'
  | 'MEM'
  | 'Main'
  | 'Sub'
  | 'TX'
  | 'RX'
  | 'MainA'
  | 'MainB'
  | 'MainC'
  | 'SubA'
  | 'SubB'
  | 'SubC'
  | 'Other';

/**
 * Radio mode type
 */
type RadioMode = 'USB' | 'LSB' | 'FM' | 'PKTFM' | 'AM' | 'CW' | 'RTTY' | 'DIG' | string;

type PassbandSelector = 'narrow' | 'wide' | 'normal' | 'nochange' | number;

interface PassbandConstants {
  NORMAL: 0;
  NOCHANGE: -1;
}

/**
 * Frequency range information
 */
interface FrequencyRange {
  /** Start frequency in Hz */
  startFreq: number;
  /** End frequency in Hz */
  endFreq: number;
  /** Supported modes for this range */
  modes: RadioMode[];
  /** Minimum power in mW */
  lowPower: number;
  /** Maximum power in mW */
  highPower: number;
  /** VFO bitmask */
  vfo: number;
  /** Antenna bitmask */
  antenna: number;
}

/**
 * Tuning step information
 */
interface TuningStepInfo {
  /** Modes this step applies to */
  modes: RadioMode[];
  /** Step size in Hz */
  stepHz: number;
}

/**
 * Filter information
 */
interface FilterInfo {
  /** Modes this filter applies to */
  modes: RadioMode[];
  /** Filter width in Hz */
  width: number;
}

interface LevelGranularityInfo {
  min: number;
  max: number;
  step: number;
  kind: 'float' | 'int';
  source: string;
}

interface RfPowerRigUnitRange {
  current: number;
  min: number;
  max: number;
}

interface RfPowerStepInfo {
  normalized: number;
  milliwatts: number;
  watts: number;
  rigUnitRange?: RfPowerRigUnitRange;
}

/**
 * Hamlib debug level type
 * Controls the verbosity of Hamlib library debug output
 */
type RigDebugLevel = 0 | 1 | 2 | 3 | 4 | 5;

type MemoryType = 'NONE' | 'MEM' | 'EDGE' | 'CALL' | 'MEMOPAD' | 'SAT' | 'BAND' | 'PRIO' | 'VOICE' | 'MORSE' | 'SPLIT';
type RepeaterShift = 'None' | 'Minus' | 'Plus' | 'NONE' | 'MINUS' | 'PLUS' | '-' | '+' | string;

interface MemoryChannelFlags {
  skip: boolean;
  data: boolean;
  pskip: boolean;
  raw: number;
}

interface MemoryCapabilities {
  bankNumber: boolean;
  vfo: boolean;
  antenna: boolean;
  frequency: boolean;
  mode: boolean;
  bandwidth: boolean;
  txFrequency: boolean;
  txMode: boolean;
  txBandwidth: boolean;
  split: boolean;
  txVfo: boolean;
  repeaterShift: boolean;
  repeaterOffset: boolean;
  tuningStep: boolean;
  rit: boolean;
  xit: boolean;
  functions: FunctionType[];
  levels: LevelType[];
  ctcssTone: boolean;
  ctcssSql: boolean;
  dcsCode: boolean;
  dcsSql: boolean;
  scanGroup: boolean;
  flags: boolean;
  description: boolean;
  tag: boolean;
}

interface MemoryRange {
  start: number;
  end: number;
  type: MemoryType;
  capabilities: MemoryCapabilities;
}

interface MemoryLayout {
  count: number;
  ranges: MemoryRange[];
}

interface MemoryChannel {
  channelNumber: number;
  bankNumber: number;
  type: MemoryType;
  empty: boolean;
  vfo: VFO | string;
  antenna: number;
  frequency: number;
  mode?: RadioMode;
  bandwidth: number;
  txFrequency: number;
  txMode?: RadioMode;
  txBandwidth: number;
  split: boolean;
  txVfo: VFO | string;
  repeaterShift: RepeaterShift;
  repeaterOffset: number;
  tuningStep: number;
  rit: number;
  xit: number;
  ctcssTone: number;
  ctcssSql: number;
  dcsCode: number;
  dcsSql: number;
  scanGroup: number;
  flags: MemoryChannelFlags;
  description: string;
  tag: string;
  functions: FunctionType[];
  levels: Partial<Record<LevelType | string, number>>;
}

type MemoryChannelInput = Partial<Omit<MemoryChannel, 'empty' | 'channelNumber' | 'flags' | 'levels'>> & {
  channelNumber: number;
  flags?: Partial<MemoryChannelFlags>;
  levels?: Partial<Record<LevelType | string, number>>;
};

/**
 * @deprecated Use MemoryChannelInput.
 */
interface MemoryChannelData extends MemoryChannelInput {}

/**
 * @deprecated Use MemoryChannel.
 */
interface MemoryChannelInfo extends MemoryChannel {}

interface MemoryListOptions {
  readOnly?: boolean;
  includeEmpty?: boolean;
  continueOnError?: boolean;
}

interface MemoryChannelError {
  channelNumber: number;
  code: number;
  message: string;
}

interface MemoryListResult {
  layout: MemoryLayout;
  channels: MemoryChannel[];
  errors: MemoryChannelError[];
}

interface MemoryWriteResult {
  written: number;
  errors: MemoryChannelError[];
}

interface MemoryFacade {
  layout(): Promise<MemoryLayout>;
  capabilities(channelNumber?: number): Promise<MemoryCapabilities>;
  count(): Promise<number>;
  get(channelNumber: number, options?: { readOnly?: boolean }): Promise<MemoryChannel>;
  set(channel: MemoryChannelInput): Promise<number>;
  list(options?: MemoryListOptions): Promise<MemoryListResult>;
  setMany(channels: MemoryChannelInput[], options?: { continueOnError?: boolean }): Promise<MemoryWriteResult>;
  replaceAll(channels: MemoryChannelInput[]): Promise<MemoryWriteResult>;
  current(vfo?: VFO): Promise<number>;
  select(channelNumber: number, vfo?: VFO): Promise<number>;
  setBank(bank: number, vfo?: VFO): Promise<number>;
}

interface SpectrumScopeInfo {
  id: number;
  name: string;
}

interface SpectrumModeInfo {
  id: number;
  name: string;
}

interface SpectrumAverageModeInfo {
  id: number;
  name: string;
}

interface SpectrumLine {
  scopeId: number;
  dataLevelMin: number;
  dataLevelMax: number;
  signalStrengthMin: number;
  signalStrengthMax: number;
  mode: number;
  centerFreq: number;
  spanHz: number;
  lowEdgeFreq: number;
  highEdgeFreq: number;
  dataLength: number;
  data: Buffer;
  timestamp: number;
}

interface SpectrumCapabilities {
  asyncDataSupported?: boolean;
  scopes: SpectrumScopeInfo[];
  modes: SpectrumModeInfo[];
  spans: number[];
  avgModes: SpectrumAverageModeInfo[];
}

interface SpectrumSupportSummary extends SpectrumCapabilities {
  supported: boolean;
  asyncDataSupported: boolean;
  hasSpectrumFunction: boolean;
  hasSpectrumHoldFunction: boolean;
  hasTransceiveFunction: boolean;
  configurableLevels: string[];
  supportsFixedEdges: boolean;
  supportsEdgeSlotSelection: boolean;
  supportedEdgeSlots: number[];
}

type SpectrumDisplayMode = 'center' | 'fixed' | 'scroll-center' | 'scroll-fixed';

interface SpectrumConfig {
  hold?: boolean;
  mode?: number | SpectrumDisplayMode;
  spanHz?: number;
  edgeSlot?: number;
  edgeLowHz?: number;
  edgeHighHz?: number;
  speed?: number;
  referenceLevel?: number;
  averageMode?: number;
}

interface SpectrumDisplayState {
  mode: SpectrumDisplayMode | null;
  modeId: number | null;
  modeName: string | null;
  spanHz: number | null;
  edgeSlot: number | null;
  edgeLowHz: number | null;
  edgeHighHz: number | null;
  supportedModes: SpectrumModeInfo[];
  supportedSpans: number[];
  supportedEdgeSlots: number[];
  supportsFixedEdges: boolean;
  supportsEdgeSlotSelection: boolean;
}

/**
 * Split mode info interface
 */
interface SplitModeInfo {
  /** TX mode */
  mode: string;
  /** TX bandwidth */
  width: number;
}

/**
 * Split status info interface
 */
interface SplitStatusInfo {
  /** Split enabled status */
  enabled: boolean;
  /** TX VFO */
  txVfo: VFO;
}

/**
 * Level type
 */
type LevelType = 'AF' | 'PREAMP' | 'ATT' | 'RF' | 'SQL' | 'RFPOWER' | 'MICGAIN' | 'IF' | 'APF' | 'NR' | 
                 'PBT_IN' | 'PBT_OUT' | 'CWPITCH' | 'KEYSPD' | 'NOTCHF' | 'COMP' | 
                 'AGC' | 'BKINDL' | 'BALANCE' | 'VOXGAIN' | 'VOXDELAY' | 'ANTIVOX' | 'MONITOR_GAIN' |
                 'STRENGTH' | 'RAWSTR' | 'SWR' | 'ALC' | 'RFPOWER_METER' | 'RFPOWER_METER_WATTS' |
                 'COMP_METER' | 'VD_METER' | 'ID_METER' | 'TEMP_METER' |
                 'SPECTRUM_MODE' | 'SPECTRUM_SPAN' | 'SPECTRUM_EDGE_LOW' | 'SPECTRUM_EDGE_HIGH' |
                 'SPECTRUM_SPEED' | 'SPECTRUM_REF' | 'SPECTRUM_AVG' | 'SPECTRUM_ATT' | string;

/**
 * Function type
 */
type FunctionType = 'FAGC' | 'NB' | 'COMP' | 'VOX' | 'TONE' | 'TSQL' | 'CSQL' | 'SBKIN' | 
                    'FBKIN' | 'ANF' | 'NR' | 'AIP' | 'APF' | 'TUNER' | 'XIT' | 
                    'RIT' | 'LOCK' | 'MUTE' | 'VSC' | 'REV' | 'SQL' | 'ABM' | 
                    'BC' | 'MBC' | 'AFC' | 'SATMODE' | 'SCOPE' | 'RESUME' | 'TRANSCEIVE' |
                    'SPECTRUM' | 'SPECTRUM_HOLD' |
                    'SEND_MORSE' | 'SEND_VOICE_MEM' | 'OVF_STATUS' |
                    'TBURST' | string;

/**
 * Scan type
 */
type ScanType = 'VFO' | 'MEM' | 'PROG' | 'DELTA' | 'PRIO';

/**
 * VFO operation type
 */
type VfoOperationType = 'CPY' | 'XCHG' | 'FROM_VFO' | 'TO_VFO' | 'MCL' | 'UP' | 
                        'DOWN' | 'BAND_UP' | 'BAND_DOWN' | 'LEFT' | 'RIGHT' | 
                        'TUNE' | 'TOGGLE';

/**
 * Supported baud rates for serial communication
 */
type SerialBaudRate = '150' | '300' | '600' | '1200' | '2400' | '4800' | '9600' | '19200' | 
                      '38400' | '57600' | '115200' | '230400' | '460800' | '500000' | 
                      '576000' | '921600' | '1000000' | '1152000' | '1500000' | '2000000' | 
                      '2500000' | '3000000' | '3500000' | '4000000';

/**
 * Serial parity values
 */
type SerialParity = 'None' | 'Even' | 'Odd' | 'Mark' | 'Space';

/**
 * Serial handshake values
 */
type SerialHandshake = 'None' | 'Hardware' | 'Software';

/**
 * Serial control signal states
 */
type SerialControlState = 'ON' | 'OFF' | 'UNSET';

/**
 * Serial configuration parameter names
 */
type SerialConfigParam = 
  // Basic serial settings
  'data_bits' | 'stop_bits' | 'serial_parity' | 'serial_handshake' |
  // Control signals
  'rts_state' | 'dtr_state' |
  // Communication settings
  'rate' | 'timeout' | 'retry' |
  // Timing control
  'write_delay' | 'post_write_delay' |
  // Advanced features
  'flushx';

/**
 * PTT (Push-to-Talk) types
 */
type PttType = 'RIG' | 'DTR' | 'RTS' | 'PARALLEL' | 'CM108' | 'GPIO' | 'GPION' | 'NONE';

/**
 * DCD (Data Carrier Detect) types
 */
type DcdType = 'RIG' | 'DSR' | 'CTS' | 'CD' | 'PARALLEL' | 'CM108' | 'GPIO' | 'GPION' | 'NONE';

/**
 * Serial configuration options interface
 */
interface SerialConfigOptions {
  /** Serial port basic parameters */
  serial: {
    /** Data bits options */
    data_bits: string[];
    /** Stop bits options */
    stop_bits: string[];
    /** Parity options */
    serial_parity: string[];
    /** Handshake options */
    serial_handshake: string[];
    /** RTS state options */
    rts_state: string[];
    /** DTR state options */
    dtr_state: string[];
  };
  /** PTT type options */
  ptt_type: string[];
  /** DCD type options */
  dcd_type: string[];
}

type HamlibConfigFieldType =
  | 'string'
  | 'combo'
  | 'numeric'
  | 'checkbutton'
  | 'button'
  | 'binary'
  | 'int'
  | 'unknown';

interface HamlibConfigFieldDescriptor {
  token: number;
  name: string;
  label: string;
  tooltip: string;
  defaultValue: string;
  type: HamlibConfigFieldType;
  numeric?: {
    min: number;
    max: number;
    step: number;
  };
  options?: string[];
}

type HamlibPortType =
  | 'none'
  | 'serial'
  | 'network'
  | 'device'
  | 'packet'
  | 'dtmf'
  | 'ultra'
  | 'rpc'
  | 'parallel'
  | 'usb'
  | 'udp-network'
  | 'cm108'
  | 'gpio'
  | 'gpion'
  | 'other';

interface HamlibPortCaps {
  portType: HamlibPortType;
  serialRateMin?: number;
  serialRateMax?: number;
  serialDataBits?: number;
  serialStopBits?: number;
  serialParity?: string;
  serialHandshake?: string;
  writeDelay: number;
  postWriteDelay: number;
  timeout: number;
  retry: number;
}

/**
 * HamLib class - for controlling amateur radio devices
 */
declare class HamLib extends EventEmitter {
  /**
   * Constructor
   * @param model Radio model number (execute rigctl -l to find your device model number)
   * @param port Optional port path
   *   - Serial connection: '/dev/ttyUSB0' (Linux default), '/dev/cu.usbserial-XXXX' (macOS), 'COM1' (Windows)
   *   - Network connection: 'localhost:4532' or '192.168.1.100:4532' (automatically switches to NETRIGCTL mode)
   * 
   * @example
   * // Use default serial port
   * const rig = new HamLib(1035);
   * 
   * // Use custom serial port
   * const rig = new HamLib(1035, '/dev/ttyUSB1');
   * 
   * // Network connection to rigctld
   * const rig = new HamLib(1035, 'localhost:4532');
   */
  constructor(model: number, port?: string);

  /** Unified memory-channel API. */
  readonly memory: MemoryFacade;

  /**
   * Get list of all supported radio models
   * @returns Array of supported radio models with details
   * @static
   * @example
   * const supportedRigs = HamLib.getSupportedRigs();
   * console.log(`Found ${supportedRigs.length} supported rigs`);
   * supportedRigs.forEach(rig => {
   *   console.log(`${rig.rigModel}: ${rig.mfgName} ${rig.modelName} (${rig.status})`);
   * });
   */
  static getSupportedRigs(): SupportedRigInfo[];

  /**
   * Get Hamlib library version information
   * @returns Hamlib version string including version number, build date, and architecture
   * @static
   * @example
   * const version = HamLib.getHamlibVersion();
   * console.log(`Hamlib version: ${version}`);
   * // Output: "Hamlib 4.5.5 2024-01-15 12:00:00 64-bit"
   */
  static getHamlibVersion(): string;

  /**
   * Set Hamlib debug level (affects all instances globally)
   * Controls the verbosity of Hamlib library debug output.
   *
   * @param level Debug level:
   *   - 0 = NONE (no debug output)
   *   - 1 = BUG (serious bug messages only)
   *   - 2 = ERR (error messages)
   *   - 3 = WARN (warning messages)
   *   - 4 = VERBOSE (verbose messages)
   *   - 5 = TRACE (trace messages - very detailed)
   * @static
   * @example
   * // Disable all debug output
   * HamLib.setDebugLevel(0);
   *
   * // Enable verbose debugging for troubleshooting
   * HamLib.setDebugLevel(4);
   *
   * // Enable full trace debugging for development
   * HamLib.setDebugLevel(5);
   */
  static setDebugLevel(level: RigDebugLevel): void;

  /**
   * Get current Hamlib debug level
   * Note: This method is not supported by Hamlib API and will throw an error.
   * Applications should track the debug level they set using setDebugLevel().
   *
   * @static
   * @throws Always throws error as Hamlib doesn't provide API to query debug level
   */
  static getDebugLevel(): never;

  /**
   * Enable or disable process-wide serialization for native HamLib radio calls.
   *
   * Enabled by default. Set NODE_HAMLIB_GLOBAL_LOCK=0, false, off, or no before
   * loading the package to start with the current no-lock behavior.
   * Async radio calls wait up to NODE_HAMLIB_GLOBAL_LOCK_TIMEOUT_MS (default 5000)
   * for this lock before rejecting with HAMLIB_GLOBAL_LOCK_TIMEOUT.
   *
   * Runtime changes only affect later lock checks; calls that have already
   * entered Hamlib are not interrupted.
   */
  static setGlobalLockEnabled(enabled: boolean): void;

  /**
   * Returns whether process-wide native HamLib radio call serialization is enabled.
   */
  static isGlobalLockEnabled(): boolean;

  /**
   * Get backend-specific configuration schema for a rig model without creating
   * a live HamLib instance or acquiring the live rig I/O lock.
   */
  static getConfigSchemaForModel(model: number): HamlibConfigFieldDescriptor[];

  /**
   * Get backend port capabilities for a rig model without creating a live
   * HamLib instance or acquiring the live rig I/O lock.
   */
  static getPortCapsForModel(model: number): HamlibPortCaps;

  /**
   * Open connection to device
   * Must be called before other operations
   * @throws Throws error when connection fails
   */
  open(): Promise<number>;

  /**
   * Set VFO (Variable Frequency Oscillator)
   * @param vfo VFO identifier, typically 'VFOA' or 'VFOB'
   * @throws Throws error when device doesn't support or operation fails
   */
  setVfo(vfo: VFO): Promise<number>;

  /**
   * Set frequency
   * @param frequency Frequency value in hertz
   * @param vfo Optional VFO to set frequency on ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @example
   * await rig.setFrequency(144390000); // Set to 144.39MHz on current VFO
   * await rig.setFrequency(144390000, 'VFOA'); // Set to 144.39MHz on VFOA
   * await rig.setFrequency(144390000, 'VFOB'); // Set to 144.39MHz on VFOB
   */
  setFrequency(frequency: number, vfo?: VFO): Promise<number>;

  /**
   * Set radio mode
   * @param mode Radio mode (such as 'USB', 'LSB', 'FM', 'PKTFM')
   * @param bandwidth Optional bandwidth selector ('narrow', 'wide', 'normal', 'nochange') or width in Hz
   * @param vfo Optional VFO to set mode on ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @example
   * await rig.setMode('USB');
   * await rig.setMode('FM', 'narrow');
   * await rig.setMode('USB', 'nochange');
   * await rig.setMode('USB', PASSBAND.NOCHANGE);
   * await rig.setMode('USB', 'wide', 'VFOA');
   */
  setMode(mode: RadioMode, bandwidth?: PassbandSelector, vfo?: VFO): Promise<number>;

  /**
   * Set PTT (Push-to-Talk) status
   * @param state true to enable PTT, false to disable PTT
   * @note Operates on the current VFO (RIG_VFO_CURR)
   */
  setPtt(state: boolean): Promise<number>;

  /**
   * Get current VFO
   * @returns Current VFO identifier
   */
  getVfo(): Promise<VFO>;

  /**
   * Get current frequency
   * @param vfo Optional VFO to get frequency from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Current frequency value in hertz
   * @example
   * await rig.getFrequency(); // Get frequency from current VFO
   * await rig.getFrequency('VFOA'); // Get frequency from VFOA
   * await rig.getFrequency('VFOB'); // Get frequency from VFOB
   */
  getFrequency(vfo?: VFO): Promise<number>;

  /**
   * Get current radio mode
   * @returns Object containing mode and bandwidth information
   * @note Operates on the current VFO (RIG_VFO_CURR)
   */
  getMode(): Promise<ModeInfo>;

  /**
   * Get current signal strength
   * @param vfo Optional VFO to get signal strength from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Signal strength value
   * @example
   * const strength = await rig.getStrength(); // Get strength from current VFO
   * const strengthA = await rig.getStrength('VFOA'); // Get strength from VFOA
   */
  getStrength(vfo?: VFO): Promise<number>;

  /**
   * Close connection to device
   * Does not destroy object, can re-establish connection by calling open()
   */
  close(): Promise<number>;

  /**
   * Destroy connection to device
   * Should delete object reference after calling to enable garbage collection
   */
  destroy(): Promise<number>;

  /**
   * Get connection information
   * @returns Object containing connection type, port path, connection status, and model numbers
   */
  getConnectionInfo(): ConnectionInfo;

  // Memory Channel Management

  /**
   * Set memory channel data
   * @deprecated Use rig.memory.set({ channelNumber, ...channelData }).
   * @param channelNumber Memory channel number
   * @param channelData Channel data (frequency, mode, description, etc.)
   */
  setMemoryChannel(channelNumber: number, channelData: MemoryChannelData): Promise<number>;

  /**
   * Get memory channel data
   * @deprecated Use rig.memory.get(channelNumber, { readOnly }).
   * @param channelNumber Memory channel number
   * @param readOnly Whether to read in read-only mode
   * @returns Channel data
   */
  getMemoryChannel(channelNumber: number, readOnly: boolean): Promise<MemoryChannelInfo>;

  /**
   * Select memory channel for operation
   * @deprecated Use rig.memory.select(channelNumber, vfo).
   * @param channelNumber Memory channel number to select
   */
  selectMemoryChannel(channelNumber: number, vfo?: VFO): Promise<number>;

  // RIT/XIT Control

  /**
   * Set RIT (Receiver Incremental Tuning) offset
   * @param offsetHz RIT offset in Hz
   */
  setRit(offsetHz: number): Promise<number>;

  /**
   * Get current RIT offset
   * @returns RIT offset in Hz
   */
  getRit(): Promise<number>;

  /**
   * Set XIT (Transmitter Incremental Tuning) offset
   * @param offsetHz XIT offset in Hz
   */
  setXit(offsetHz: number): Promise<number>;

  /**
   * Get current XIT offset
   * @returns XIT offset in Hz
   */
  getXit(): Promise<number>;

  /**
   * Clear both RIT and XIT offsets
   * @returns Success status
   */
  clearRitXit(): Promise<number>;

  // Scanning Operations

  /**
   * Start scanning operation
   * @param scanType Scan type ('VFO', 'MEM', 'PROG', 'DELTA', 'PRIO')
   * @param scanType Scan type ('VFO', 'MEM', 'PROG', 'DELTA', 'PRIO')
   * @param channel Channel number for some scan types
   */
  startScan(scanType: ScanType, channel: number): Promise<number>;

  /**
   * Stop scanning operation
   */
  stopScan(): Promise<number>;

  // Level Controls

  /**
   * Set radio level (gain, volume, etc.)
   * @param levelType Level type ('AF', 'RF', 'SQL', 'RFPOWER', etc.)
   * @param value Level value (0.0-1.0 typically)
   */
  setLevel(levelType: LevelType, value: number, vfo?: VFO): Promise<number>;

  /**
   * Get radio level
   * @param levelType Level type ('AF', 'RF', 'SQL', 'STRENGTH', etc.)
   * @param vfo Optional Hamlib VFO token. If not specified, uses current VFO.
   * @returns Level value
   */
  getLevel(levelType: LevelType, vfo?: VFO): Promise<number>;

  /**
   * Get list of supported level types
   * @returns Array of supported level types
   */
  getSupportedLevels(): Promise<string[]>;

  // Function Controls

  /**
   * Set radio function on/off
   * @param functionType Function type ('NB', 'COMP', 'VOX', 'TONE', etc.)
   * @param enable true to enable, false to disable
   */
  setFunction(functionType: FunctionType, enable: boolean): Promise<number>;

  /**
   * Get radio function status
   * @param functionType Function type ('NB', 'COMP', 'VOX', 'TONE', etc.)
   * @returns Function enabled status
   */
  getFunction(functionType: FunctionType): Promise<boolean>;

  /**
   * Get list of supported function types
   * @returns Array of supported function types
   */
  getSupportedFunctions(): Promise<string[]>;

  /**
   * Get list of supported radio modes
   * @returns Array of supported mode strings
   * @example
   * const modes = rig.getSupportedModes();
   * console.log('Supported modes:', modes); // ['USB', 'LSB', 'CW', 'FM', 'AM', ...]
   */
  getSupportedModes(): Promise<string[]>;

  // Split Operations

  /**
   * Set split mode TX frequency
   * @param txFrequency TX frequency in Hz
   * @param vfo Optional VFO to set TX frequency on ('VFOA' or 'VFOB'). If not specified, uses current VFO
   */
  setSplitFreq(txFrequency: number, vfo?: VFO): Promise<number>;

  /**
   * Get split mode TX frequency
   * @param vfo Optional VFO to get TX frequency from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns TX frequency in Hz
   */
  getSplitFreq(vfo?: VFO): Promise<number>;

  /**
   * Set split mode TX mode
   * @param txMode TX mode ('USB', 'LSB', 'FM', etc.)
   * @param vfo Optional VFO to set TX mode on ('VFOA' or 'VFOB'). If not specified, uses current VFO
   */
  setSplitMode(txMode: RadioMode, vfo?: VFO): Promise<number>;

  /**
   * Set split mode TX mode with width
   * @param txMode TX mode ('USB', 'LSB', 'FM', etc.)
   * @param txWidth TX bandwidth
   * @param vfo Optional VFO to set TX mode on ('VFOA' or 'VFOB'). If not specified, uses current VFO
   */
  setSplitMode(txMode: RadioMode, txWidth: number, vfo?: VFO): Promise<number>;

  /**
   * Get split mode TX mode
   * @param vfo Optional VFO to get TX mode from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns TX mode and width
   */
  getSplitMode(vfo?: VFO): Promise<SplitModeInfo>;

  /**
   * Enable/disable split operation
   * @param enable true to enable split, false to disable
   * @param rxVfo Optional RX VFO ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @param txVfo Optional TX VFO ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @example
   * await rig.setSplit(true);                    // Enable split with defaults
   * await rig.setSplit(true, 'VFOA');          // Enable split, RX on VFOA, TX on current VFO
   * await rig.setSplit(true, 'VFOA', 'VFOB'); // Enable split, explicit RX and TX VFOs
   */
  setSplit(enable: boolean, rxVfo?: VFO, txVfo?: VFO): Promise<number>;

  /**
   * Get split operation status
   * @param vfo Optional VFO to get status from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Split status and TX VFO
   */
  getSplit(vfo?: VFO): Promise<SplitStatusInfo>;

  // VFO Operations

  /**
   * Perform VFO operation
   * @param operation VFO operation ('CPY', 'XCHG', 'FROM_VFO', 'TO_VFO', etc.)
   */
  vfoOperation(operation: VfoOperationType): Promise<number>;

  // Antenna Selection

  // Note: setAntenna and getAntenna are defined later with full VFO support

  // Serial Port Configuration

  /**
   * Set serial port configuration parameter
   * @param paramName Parameter name for serial configuration
   * @param paramValue Parameter value as string
   * @example
   * // Basic serial settings
   * await hamlib.setSerialConfig('data_bits', '8');
   * await hamlib.setSerialConfig('stop_bits', '1');
   * await hamlib.setSerialConfig('serial_parity', 'Even');
   * await hamlib.setSerialConfig('serial_handshake', 'Hardware');
   * 
   * // Control signals
   * await hamlib.setSerialConfig('rts_state', 'ON');
   * await hamlib.setSerialConfig('dtr_state', 'OFF');
   * 
   * // Communication settings
   * await hamlib.setSerialConfig('rate', '115200');
   * await hamlib.setSerialConfig('timeout', '1000');
   * await hamlib.setSerialConfig('retry', '3');
   * 
   * // Timing control
   * await hamlib.setSerialConfig('write_delay', '10');
   * await hamlib.setSerialConfig('post_write_delay', '50');
   * 
   * // Advanced features
   * await hamlib.setSerialConfig('flushx', 'true');
   */
  setSerialConfig(paramName: SerialConfigParam, paramValue: string): Promise<number>;

  /**
   * Get serial port configuration parameter
   * @param paramName Parameter name to retrieve (any SerialConfigParam)
   * @returns Parameter value as string
   * @example
   * // Get basic serial settings
   * const dataBits = await hamlib.getSerialConfig('data_bits');
   * const parity = await hamlib.getSerialConfig('serial_parity');
   * const handshake = await hamlib.getSerialConfig('serial_handshake');
   * 
   * // Get communication settings
   * const rate = await hamlib.getSerialConfig('rate');
   * const timeout = await hamlib.getSerialConfig('timeout');
   * const retry = await hamlib.getSerialConfig('retry');
   * 
   * // Get control signals
   * const rtsState = await hamlib.getSerialConfig('rts_state');
   * const dtrState = await hamlib.getSerialConfig('dtr_state');
   * 
   * // Get timing and advanced settings
   * const writeDelay = await hamlib.getSerialConfig('write_delay');
   * const flushx = await hamlib.getSerialConfig('flushx');
   */
  getSerialConfig(paramName: SerialConfigParam): Promise<string>;

  /**
   * Set PTT (Push-to-Talk) type
   * @param pttType PTT type ('RIG', 'DTR', 'RTS', 'PARALLEL', 'CM108', 'GPIO', 'GPION', 'NONE')
   * @example
   * // Use DTR line for PTT
   * await hamlib.setPttType('DTR');
   * 
   * // Use RTS line for PTT
   * await hamlib.setPttType('RTS');
   * 
   * // Use CAT command for PTT
   * await hamlib.setPttType('RIG');
   */
  setPttType(pttType: PttType): Promise<number>;

  /**
   * Get current PTT type
   * @returns Current PTT type
   */
  getPttType(): Promise<string>;

  /**
   * Set DCD (Data Carrier Detect) type
   * @param dcdType DCD type ('RIG', 'DSR', 'CTS', 'CD', 'PARALLEL', 'CM108', 'GPIO', 'GPION', 'NONE')
   * @example
   * // Use DSR line for DCD
   * await hamlib.setDcdType('DSR');
   * 
   * // Use CTS line for DCD
   * await hamlib.setDcdType('CTS');
   * 
   * // Use CAT command for DCD
   * await hamlib.setDcdType('RIG');
   */
  setDcdType(dcdType: DcdType): Promise<number>;

  /**
   * Get current DCD type
   * @returns Current DCD type
   */
  getDcdType(): Promise<string>;

  /**
   * Get supported serial configuration options
   * @returns Object containing all supported configuration parameters and their possible values
   * @example
   * const configs = hamlib.getSupportedSerialConfigs();
   * console.log('Supported data bits:', configs.serial.data_bits);
   * console.log('Supported PTT types:', configs.ptt_type);
   */
  getSupportedSerialConfigs(): SerialConfigOptions;

  /**
   * Get backend-specific configuration schema for the current rig model.
   * This can be queried before opening the connection and is suitable for
   * driving dynamic configuration UIs.
   */
  getConfigSchema(): Promise<HamlibConfigFieldDescriptor[]>;

  /**
   * Get backend port capabilities and effective connection defaults.
   */
  getPortCaps(): Promise<HamlibPortCaps>;

  // Power Control

  /**
   * Set radio power status
   * @param status Power status (0=OFF, 1=ON, 2=STANDBY, 4=OPERATE, 8=UNKNOWN)
   * @returns Success status
   * @example
   * await rig.setPowerstat(1); // Power on
   * await rig.setPowerstat(0); // Power off
   * await rig.setPowerstat(2); // Standby mode
   */
  setPowerstat(status: number): Promise<number>;

  /**
   * Get current radio power status
   * @returns Power status (0=OFF, 1=ON, 2=STANDBY, 4=OPERATE, 8=UNKNOWN)
   * @example
   * const powerStatus = await rig.getPowerstat();
   * if (powerStatus === 1) {
   *   console.log('Radio is powered on');
   * }
   */
  getPowerstat(): Promise<number>;

  // PTT Status Detection

  /**
   * Get current PTT status
   * @param vfo Optional VFO to get PTT status from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns PTT status (true if transmitting, false if receiving)
   * @example
   * const isTransmitting = await rig.getPtt();
   * if (isTransmitting) {
   *   console.log('Radio is transmitting');
   * }
   */
  getPtt(vfo?: VFO): Promise<boolean>;

  // Data Carrier Detect

  /**
   * Get current DCD (Data Carrier Detect) status
   * @param vfo Optional VFO to get DCD status from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns DCD status (true if carrier detected, false if no carrier)
   * @example
   * const carrierDetected = await rig.getDcd();
   * if (carrierDetected) {
   *   console.log('Carrier detected');
   * }
   */
  getDcd(vfo?: VFO): Promise<boolean>;

  // Tuning Step Control

  /**
   * Set tuning step
   * @param step Tuning step in Hz
   * @param vfo Optional VFO to set tuning step on ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status
   * @example
   * await rig.setTuningStep(25000); // 25 kHz steps
   * await rig.setTuningStep(12500); // 12.5 kHz steps
   * await rig.setTuningStep(100);   // 100 Hz steps for HF
   */
  setTuningStep(step: number, vfo?: VFO): Promise<number>;

  /**
   * Get current tuning step
   * @param vfo Optional VFO to get tuning step from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Current tuning step in Hz
   * @example
   * const step = await rig.getTuningStep();
   * console.log(`Current tuning step: ${step} Hz`);
   */
  getTuningStep(vfo?: VFO): Promise<number>;

  // Repeater Control

  /**
   * Set repeater shift direction
   * @param shift Repeater shift direction ('NONE', 'MINUS', 'PLUS', '-', '+')
   * @param vfo Optional VFO to set repeater shift on ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status
   * @example
   * await rig.setRepeaterShift('PLUS');  // Positive shift (+)
   * await rig.setRepeaterShift('MINUS'); // Negative shift (-)
   * await rig.setRepeaterShift('NONE');  // No shift (simplex)
   */
  setRepeaterShift(shift: 'NONE' | 'MINUS' | 'PLUS' | '-' | '+' | 'none' | 'minus' | 'plus', vfo?: VFO): Promise<number>;

  /**
   * Get current repeater shift direction
   * @param vfo Optional VFO to get repeater shift from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Current repeater shift direction
   * @example
   * const shift = await rig.getRepeaterShift();
   * console.log(`Repeater shift: ${shift}`); // 'None', 'Minus', or 'Plus'
   */
  getRepeaterShift(vfo?: VFO): Promise<string>;

  /**
   * Set repeater offset frequency
   * @param offset Repeater offset in Hz
   * @param vfo Optional VFO to set repeater offset on ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status
   * @example
   * await rig.setRepeaterOffset(600000);  // 600 kHz offset (2m band)
   * await rig.setRepeaterOffset(5000000); // 5 MHz offset (70cm band)
   */
  setRepeaterOffset(offset: number, vfo?: VFO): Promise<number>;

  /**
   * Get current repeater offset frequency
   * @param vfo Optional VFO to get repeater offset from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Current repeater offset in Hz
   * @example
   * const offset = await rig.getRepeaterOffset();
   * console.log(`Repeater offset: ${offset} Hz`);
   */
  getRepeaterOffset(vfo?: VFO): Promise<number>;

  // CTCSS/DCS Tone Control
  
  /**
   * Set CTCSS tone frequency
   * @param tone CTCSS tone frequency in tenths of Hz (e.g., 1000 for 100.0 Hz)
   * @param vfo Optional VFO to set tone on ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status
   * @example
   * await rig.setCtcssTone(1000); // Set 100.0 Hz CTCSS tone
   * await rig.setCtcssTone(1318); // Set 131.8 Hz CTCSS tone
   */
  setCtcssTone(tone: number, vfo?: VFO): Promise<number>;

  /**
   * Get current CTCSS tone frequency
   * @param vfo Optional VFO to get tone from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Current CTCSS tone frequency in tenths of Hz
   * @example
   * const tone = await rig.getCtcssTone();
   * console.log(`CTCSS tone: ${tone / 10} Hz`);
   */
  getCtcssTone(vfo?: VFO): Promise<number>;

  /**
   * Set DCS code
   * @param code DCS code (e.g., 23, 174, 754)
   * @param vfo Optional VFO to set code on ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status
   * @example
   * await rig.setDcsCode(23);  // Set DCS code 023
   * await rig.setDcsCode(174); // Set DCS code 174
   */
  setDcsCode(code: number, vfo?: VFO): Promise<number>;

  /**
   * Get current DCS code
   * @param vfo Optional VFO to get code from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Current DCS code
   * @example
   * const code = await rig.getDcsCode();
   * console.log(`DCS code: ${code.toString().padStart(3, '0')}`);
   */
  getDcsCode(vfo?: VFO): Promise<number>;

  /**
   * Set CTCSS SQL (squelch) tone frequency
   * @param tone CTCSS SQL tone frequency in tenths of Hz (e.g., 1000 for 100.0 Hz)
   * @param vfo Optional VFO to set tone on ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status
   * @example
   * await rig.setCtcssSql(1000); // Set 100.0 Hz CTCSS SQL tone
   */
  setCtcssSql(tone: number, vfo?: VFO): Promise<number>;

  /**
   * Get current CTCSS SQL tone frequency
   * @param vfo Optional VFO to get tone from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Current CTCSS SQL tone frequency in tenths of Hz
   * @example
   * const tone = await rig.getCtcssSql();
   * console.log(`CTCSS SQL tone: ${tone / 10} Hz`);
   */
  getCtcssSql(vfo?: VFO): Promise<number>;

  /**
   * Set DCS SQL (squelch) code
   * @param code DCS SQL code (e.g., 23, 174, 754)
   * @param vfo Optional VFO to set code on ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status
   * @example
   * await rig.setDcsSql(23); // Set DCS SQL code 023
   */
  setDcsSql(code: number, vfo?: VFO): Promise<number>;

  /**
   * Get current DCS SQL code
   * @param vfo Optional VFO to get code from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Current DCS SQL code
   * @example
   * const code = await rig.getDcsSql();
   * console.log(`DCS SQL code: ${code.toString().padStart(3, '0')}`);
   */
  getDcsSql(vfo?: VFO): Promise<number>;

  // Parameter Control

  /**
   * Set radio parameter
   * @param paramName Parameter name (e.g., 'ANN', 'APO', 'BACKLIGHT', 'BEEP', 'TIME', 'BAT')
   * @param value Parameter value (numeric)
   * @returns Success status
   * @example
   * await rig.setParm('BACKLIGHT', 0.5); // Set backlight to 50%
   * await rig.setParm('BEEP', 1);        // Enable beep
   */
  setParm(paramName: string, value: number): Promise<number>;

  /**
   * Get radio parameter value
   * @param paramName Parameter name (e.g., 'ANN', 'APO', 'BACKLIGHT', 'BEEP', 'TIME', 'BAT')
   * @returns Parameter value
   * @example
   * const backlight = await rig.getParm('BACKLIGHT');
   * const beep = await rig.getParm('BEEP');
   */
  getParm(paramName: string): Promise<number>;

  // DTMF Support

  /**
   * Send DTMF digits
   * @param digits DTMF digits to send (0-9, A-D, *, #)
   * @param vfo Optional VFO to send from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status
   * @example
   * await rig.sendDtmf('1234'); // Send DTMF sequence 1234
   * await rig.sendDtmf('*70#'); // Send *70# sequence
   */
  sendDtmf(digits: string, vfo?: VFO): Promise<number>;

  /**
   * Receive DTMF digits
   * @param maxLength Maximum number of digits to receive
   * @param vfo Optional VFO to receive from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Received DTMF digits and count
   * @example
   * const result = await rig.recvDtmf(10);
   * console.log(`Received DTMF: ${result.digits} (${result.length} digits)`);
   */
  recvDtmf(maxLength: number, vfo?: VFO): Promise<{digits: string, length: number}>;

  // Memory Channel Advanced Operations

  /**
   * Get current memory channel number
   * @deprecated Use rig.memory.current(vfo).
   * @param vfo Optional VFO to get memory channel from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Current memory channel number
   * @example
   * const currentChannel = await rig.getMem();
   * console.log(`Current memory channel: ${currentChannel}`);
   */
  getMem(vfo?: VFO): Promise<number>;

  /**
   * Set memory bank
   * @deprecated Use rig.memory.setBank(bank, vfo).
   * @param bank Bank number to select
   * @param vfo Optional VFO to set bank on ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status
   * @example
   * await rig.setBank(1); // Select memory bank 1
   * await rig.setBank(2); // Select memory bank 2
   */
  setBank(bank: number, vfo?: VFO): Promise<number>;

  /**
   * Get total number of memory channels available
   * @deprecated Use rig.memory.count().
   * @returns Total number of memory channels
   * @example
   * const totalChannels = await rig.memCount();
   * console.log(`Total memory channels: ${totalChannels}`);
   */
  memCount(): Promise<number>;

  // Morse Code Support

  /**
   * Send Morse code message
   * @param message Morse code message to send (text will be converted to Morse)
   * @param vfo Optional VFO to send from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status
   * @example
   * await rig.sendMorse('CQ CQ DE VK3ABC K'); // Send CQ call
   * await rig.sendMorse('TEST MSG');          // Send test message
   */
  sendMorse(message: string, vfo?: VFO): Promise<number>;

  /**
   * Stop current Morse code transmission
   * @param vfo Optional VFO to stop ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status
   * @example
   * await rig.stopMorse(); // Stop ongoing Morse transmission
   */
  stopMorse(vfo?: VFO): Promise<number>;

  /**
   * Wait for Morse code transmission to complete
   * @param vfo Optional VFO to wait on ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status when transmission completes
   * @example
   * await rig.sendMorse('TEST');
   * await rig.waitMorse(); // Wait for transmission to complete
   */
  waitMorse(vfo?: VFO): Promise<number>;

  // Voice Memory Support

  /**
   * Play voice memory channel
   * @param channel Voice memory channel number to play
   * @param vfo Optional VFO to play from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status
   * @example
   * await rig.sendVoiceMem(1); // Play voice memory channel 1
   * await rig.sendVoiceMem(3); // Play voice memory channel 3
   */
  sendVoiceMem(channel: number, vfo?: VFO): Promise<number>;

  /**
   * Stop voice memory playback
   * @param vfo Optional VFO to stop ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status
   * @example
   * await rig.stopVoiceMem(); // Stop ongoing voice memory playback
   */
  stopVoiceMem(vfo?: VFO): Promise<number>;

  // Complex Split Frequency/Mode Operations

  /**
   * Set split frequency and mode simultaneously
   * @param txFrequency TX frequency in Hz
   * @param txMode TX mode ('USB', 'LSB', 'FM', etc.)
   * @param txWidth TX bandwidth in Hz
   * @param vfo Optional VFO for split operation ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status
   * @example
   * await rig.setSplitFreqMode(14205000, 'USB', 2400); // Set split: TX on 14.205 MHz USB 2400Hz width
   * await rig.setSplitFreqMode(145520000, 'FM', 15000); // Set split: TX on 145.52 MHz FM 15kHz width
   */
  setSplitFreqMode(txFrequency: number, txMode: RadioMode, txWidth: number, vfo?: VFO): Promise<number>;

  /**
   * Get split frequency and mode information
   * @param vfo Optional VFO to get split info from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Object containing TX frequency, mode, and width
   * @example
   * const splitInfo = await rig.getSplitFreqMode();
   * console.log(`Split TX: ${splitInfo.txFrequency} Hz, Mode: ${splitInfo.txMode}, Width: ${splitInfo.txWidth} Hz`);
   */
  getSplitFreqMode(vfo?: VFO): Promise<{txFrequency: number, txMode: string, txWidth: number}>;

  // Antenna Control

  /**
   * Set antenna selection
   * @param antenna Antenna number to select (1-based)
   * @param vfo Optional VFO to set antenna on ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Success status
   * @example
   * await rig.setAntenna(1); // Select antenna 1
   * await rig.setAntenna(2); // Select antenna 2
   */
  setAntenna(antenna: number, vfo?: VFO): Promise<number>;
  setAntenna(antenna: number, option: number, vfo?: VFO): Promise<number>;

  /**
   * Get comprehensive antenna information
   * @param vfo Optional VFO to get antenna info from ('VFOA' or 'VFOB'). If not specified, uses current VFO
   * @returns Antenna information object containing current, TX, RX antenna settings and option
   * @example
   * const antennaInfo = await rig.getAntenna();
   * console.log(`Current: ${antennaInfo.currentAntenna}`);
   * console.log(`TX: ${antennaInfo.txAntenna}, RX: ${antennaInfo.rxAntenna}`);
   * console.log(`Option: ${antennaInfo.option}`);
   */
  getAntenna(vfo?: VFO): Promise<AntennaInfo>;

  // Power Conversion Functions

  /**
   * Convert power level to milliwatts
   * @param power Power level (0.0-1.0, where 1.0 = 100% power)
   * @param frequency Frequency in Hz
   * @param mode Radio mode ('USB', 'LSB', 'FM', 'AM', etc.)
   * @returns Power in milliwatts
   * @example
   * const milliwatts = await rig.power2mW(0.5, 14205000, 'USB');
   * console.log(`50% power = ${milliwatts} mW`);
   */
  power2mW(power: number, frequency: number, mode: RadioMode): Promise<number>;

  /**
   * Convert milliwatts to power level
   * @param milliwatts Power in milliwatts
   * @param frequency Frequency in Hz
   * @param mode Radio mode ('USB', 'LSB', 'FM', 'AM', etc.)
   * @returns Power level (0.0-1.0, where 1.0 = 100% power)
   * @example
   * const powerLevel = await rig.mW2power(5000, 14205000, 'USB');
   * console.log(`5000 mW = ${(powerLevel * 100).toFixed(1)}% power`);
   */
  mW2power(milliwatts: number, frequency: number, mode: RadioMode): Promise<number>;

  // Reset Function

  /**
   * Reset radio to default state
   * @param resetType Reset type ('NONE', 'SOFT', 'VFO', 'MCALL', 'MASTER')
   *   - NONE: No reset
   *   - SOFT: Soft reset (preserve some settings)
   *   - VFO: VFO reset
   *   - MCALL: Memory clear
   *   - MASTER: Master reset (complete factory reset)
   * @returns Success status
   * @example
   * await rig.reset('SOFT');   // Soft reset
   * await rig.reset('MASTER'); // Factory reset (CAUTION: loses all settings!)
   */
  reset(resetType: 'NONE' | 'SOFT' | 'VFO' | 'MCALL' | 'MASTER'): Promise<number>;

  // ===== Rig Info / Spectrum / Conf (async) =====

  /**
   * Get rig identification info (model, firmware version, etc.)
   * @returns Rig info string from the backend
   */
  getInfo(): Promise<string>;

  /**
   * Send raw command bytes and receive reply
   * @param data Raw command data to send
   * @param replyMaxLen Reply buffer length in bytes (2-200 for request/response, 0 for write-only)
   * @param terminator Optional terminator bytes
   * @returns Reply data as Buffer
   */
  sendRaw(data: Buffer, replyMaxLen: number, terminator?: Buffer): Promise<Buffer>;

  /** Send raw command bytes without reading a reply. */
  sendRawWrite(data: Buffer): Promise<void>;

  /**
   * Get official Hamlib spectrum metadata exposed by the backend.
   */
  getSpectrumCapabilities(): Promise<SpectrumCapabilities>;

  /**
   * Start receiving official Hamlib spectrum line events.
   */
  startSpectrumStream(callback?: (line: SpectrumLine) => void): Promise<boolean>;

  /**
   * Stop receiving official Hamlib spectrum line events.
   */
  stopSpectrumStream(): Promise<boolean>;

  /**
   * Listen for official spectrum line events.
   */
  on(event: 'spectrumLine', listener: (line: SpectrumLine) => void): this;
  once(event: 'spectrumLine', listener: (line: SpectrumLine) => void): this;
  off(event: 'spectrumLine', listener: (line: SpectrumLine) => void): this;

  /**
   * Listen for asynchronous spectrum errors.
   */
  on(event: 'spectrumError', listener: (error: Error) => void): this;
  once(event: 'spectrumError', listener: (error: Error) => void): this;
  off(event: 'spectrumError', listener: (error: Error) => void): this;

  /**
   * Set a backend configuration parameter
   * @param name Configuration token name
   * @param value Configuration value
   * @returns Success status
   */
  setConf(name: string, value: string): Promise<number>;

  /**
   * Get a backend configuration parameter
   * @param name Configuration token name
   * @returns Configuration value
   */
  getConf(name: string): Promise<string>;

  // ===== Passband / Resolution (async) =====

  /**
   * Get normal passband width for a given mode
   * @param mode Radio mode
   * @returns Normal passband width in Hz
   */
  getPassbandNormal(mode: RadioMode): Promise<number>;

  /**
   * Get narrow passband width for a given mode
   * @param mode Radio mode
   * @returns Narrow passband width in Hz
   */
  getPassbandNarrow(mode: RadioMode): Promise<number>;

  /**
   * Get wide passband width for a given mode
   * @param mode Radio mode
   * @returns Wide passband width in Hz
   */
  getPassbandWide(mode: RadioMode): Promise<number>;

  /**
   * Get frequency resolution (step) for a given mode
   * @param mode Radio mode
   * @returns Resolution in Hz
   */
  getResolution(mode: RadioMode): Promise<number>;

  // ===== Capability queries (async) =====

  /**
   * Get list of supported parameter types
   * @returns Array of supported parameter type names
   */
  getSupportedParms(): Promise<string[]>;

  /**
   * Get list of supported VFO operations
   * @returns Array of supported VFO operation types
   */
  getSupportedVfoOps(): Promise<VfoOperationType[]>;

  /**
   * Get list of supported scan types
   * @returns Array of supported scan types
   */
  getSupportedScanTypes(): Promise<ScanType[]>;

  // ===== Capability Queries - Batch 2 (async) =====

  /**
   * Get supported preamplifier values
   * @returns Array of preamp dB values
   */
  getPreampValues(): Promise<number[]>;

  /**
   * Get supported attenuator values
   * @returns Array of attenuator dB values
   */
  getAttenuatorValues(): Promise<number[]>;

  /**
   * Get supported AGC mode values
   * @returns Array of Hamlib AGC enum values
   */
  getAgcLevels(): Promise<number[]>;

  /**
   * Get maximum RIT offset supported
   * @returns Maximum RIT offset in Hz
   */
  getMaxRit(): Promise<number>;

  /**
   * Get maximum XIT offset supported
   * @returns Maximum XIT offset in Hz
   */
  getMaxXit(): Promise<number>;

  /**
   * Get maximum IF shift supported
   * @returns Maximum IF shift in Hz
   */
  getMaxIfShift(): Promise<number>;

  /**
   * Get list of available CTCSS tone frequencies
   * @returns Array of CTCSS tones in Hz
   */
  getAvailableCtcssTones(): Promise<number[]>;

  /**
   * Get list of available DCS codes
   * @returns Array of DCS codes
   */
  getAvailableDcsCodes(): Promise<number[]>;

  /**
   * Get supported frequency ranges for RX and TX
   */
  getFrequencyRanges(): Promise<{ rx: FrequencyRange[]; tx: FrequencyRange[] }>;

  /**
   * Get supported tuning steps per mode
   */
  getTuningSteps(): Promise<TuningStepInfo[]>;

  /**
   * Get supported filter widths per mode
   */
  getFilterList(): Promise<FilterInfo[]>;

  /**
   * Get granularity metadata for a level setting from Hamlib rig state.
   */
  getLevelGranularity(levelType: LevelType): Promise<LevelGranularityInfo | null>;

  /**
   * Get a discrete RF power step table for the current rig, frequency, and mode.
   */
  getRfPowerStepTable(frequency: number, mode: RadioMode): Promise<RfPowerStepInfo[] | null>;

  // ===== Static: Copyright / License =====

  /**
   * Get Hamlib copyright information
   * @static
   */
  static getCopyright(): string;

  /**
   * Get Hamlib license information
   * @static
   */
  static getLicense(): string;

  // ===== Lock Mode (Hamlib >= 4.7.0) =====

  /**
   * Set lock mode to prevent accidental front-panel changes
   * @param lock - 0 to unlock, 1 to lock
   */
  setLockMode(lock: number): Promise<number>;

  /**
   * Get current lock mode status
   * @returns 0 if unlocked, 1 if locked
   */
  getLockMode(): Promise<number>;

  // ===== Clock (Hamlib >= 4.7.0) =====

  /**
   * Set the rig's internal clock
   */
  setClock(clock: ClockInfo): Promise<number>;

  /**
   * Get the rig's internal clock
   */
  getClock(): Promise<ClockInfo>;

  // ===== VFO Info (Hamlib >= 4.7.0) =====

  /**
   * Get comprehensive VFO information in a single call
   * @param vfo - Optional VFO to query (defaults to current)
   */
  getVfoInfo(vfo?: VFO): Promise<VfoInfo>;
}

declare class Rotator extends EventEmitter {
  constructor(model: number, port?: string);

  static getSupportedRotators(): SupportedRotatorInfo[];
  static getHamlibVersion(): string;
  static setDebugLevel(level: RigDebugLevel): void;
  static getDebugLevel(): never;
  static getCopyright(): string;
  static getLicense(): string;

  open(): Promise<number>;
  close(): Promise<number>;
  destroy(): Promise<number>;
  getConnectionInfo(): RotatorConnectionInfo;

  setPosition(azimuth: number, elevation: number): Promise<number>;
  getPosition(): Promise<RotatorPosition>;
  move(direction: RotatorDirection, speed: number): Promise<number>;
  stop(): Promise<number>;
  park(): Promise<number>;
  reset(resetType: RotatorResetType): Promise<number>;

  getInfo(): Promise<string>;
  getStatus(): Promise<RotatorStatus>;

  setConf(name: string, value: string): Promise<number>;
  getConf(name: string): Promise<string>;
  getConfigSchema(): HamlibConfigFieldDescriptor[];
  getPortCaps(): HamlibPortCaps;
  getRotatorCaps(): RotatorCaps;

  setLevel(level: string, value: number): Promise<number>;
  getLevel(level: string): Promise<number>;
  getSupportedLevels(): string[];

  setFunction(func: string, enable: boolean): Promise<number>;
  getFunction(func: string): Promise<boolean>;
  getSupportedFunctions(): string[];

  setParm(parm: string, value: number): Promise<number>;
  getParm(parm: string): Promise<number>;
  getSupportedParms(): string[];
}

/**
 * Clock information for rig's internal clock
 */
interface ClockInfo {
  year: number;
  month: number;
  day: number;
  hour: number;
  min: number;
  sec: number;
  msec: number;
  utcOffset: number;
}

/**
 * VFO information returned by getVfoInfo()
 */
interface VfoInfo {
  frequency: number;
  mode: string;
  bandwidth: number;
  split: boolean;
  satMode: boolean;
}

/**
 * hamlib module export object
 */
declare const nodeHamlib: {
  HamLib: typeof HamLib;
  Rotator: typeof Rotator;
  PASSBAND: PassbandConstants;
};

declare const PASSBAND: PassbandConstants;

// Export types for use elsewhere
export { ConnectionInfo, ModeInfo, SupportedRigInfo, SupportedRotatorInfo, AntennaInfo, RotatorConnectionInfo,
         RotatorPosition, RotatorStatus, RotatorDirection, RotatorResetType, RotatorCaps, VFO, RadioMode, MemoryChannelData,
         MemoryChannelInfo, MemoryType, RepeaterShift, MemoryChannelFlags, MemoryCapabilities, MemoryRange,
         MemoryLayout, MemoryChannel, MemoryChannelInput, MemoryListOptions, MemoryChannelError,
         MemoryListResult, MemoryWriteResult, MemoryFacade, SplitModeInfo, SplitStatusInfo, LevelType, FunctionType,
         ScanType, VfoOperationType, SerialConfigParam, SerialBaudRate, SerialParity,
         SerialHandshake, SerialControlState, PttType, DcdType, SerialConfigOptions,
         HamlibConfigFieldType, HamlibConfigFieldDescriptor, HamlibPortType, HamlibPortCaps,
         SpectrumScopeInfo, SpectrumModeInfo, SpectrumAverageModeInfo, SpectrumLine,
         SpectrumCapabilities, SpectrumSupportSummary, SpectrumConfig, SpectrumDisplayState,
         ClockInfo, VfoInfo, PassbandSelector, PassbandConstants, HamLib, Rotator, PASSBAND };

// Support both CommonJS and ES module exports
// @ts-ignore
export = nodeHamlib;
export default nodeHamlib; 
