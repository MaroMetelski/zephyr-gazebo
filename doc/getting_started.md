# Getting started

## Zephyr set-up

Make sure you have all the Zephyr prerequisites in place: system dependencies,
python packages and the SDK. It's assumed the users already know Zephyr basics
and undestand how to set up a project and use external modules.
If you need help with that, visit Zephyr documentation.

### As a module

The project can be included as a Zephyr module into an existing workspace.
Add the project to the west manifest and update the workspace.

> Note you will also need the following modules in your workspace:
> - **nanopb - required**
> - cmsis-dsp (optional, used when building certain example applications)

```yaml
projects:
  - name: zephyr_gazebo
    url: https://github.com/MaroMetelski/zephyr-gazebo.git
```

### Standalone

The project can be used standalone (for example when working on the `zephyr-gazebo` project itself)
In that case set up the west workspace with `zephyr-gazebo` as base:

```bash
west init -m https://github.com/MaroMetelski/zephyr-gazebo.git
west update
```

## Gazebo set-up

Currently the project supports Gazebo **Jetty** on Ubuntu **26.04 LTS**.

The project depends on `python3-gz-transport15` and `libgz-msgs12-dev` packages,
which should be already installed if you have full Gazebo installation on your system.
For instruction on installing Gazebo Jetty, refer to [official documentation](https://gazebosim.org/docs/latest/install/).

> NOTE: If you are using Python virtual enviroments, you may need to give it access to
 system packages to access `python3-gz-transport15`, for example:
 `python3 -m venv --system-site-packages <venv_directory>`

## Balance bot example

### Running Gazebo

Start the gazebo world in the example directory:

```bash
cd samples/balance_bot
gz sim balance_bot.sdf
```

### Running the bridge script

Start the bridge python script

```bash
cd scripts/gz_bridge
python3 gz_bridge.py \
  --dts-file ../../build/zephyr/zephyr.dts \
  --map-file ../../samples/balance_bot/bridge_map.yaml
```

### Running the example application

```bash
west build -p -b native_sim samples/balance_bot
west build -t run
```

### Run the simulation

Click the "play" button in the Gazebo window.
