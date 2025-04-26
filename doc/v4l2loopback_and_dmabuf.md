# v4l2loopback and DMABUF

This document describes the video data flow on Linux using the `v4l2loopback` kernel module, leveraging DMABUF for efficient buffer sharing between the DMABUF producer (`v4l2_clone_device`) and the DMABUF consumer (`v4l2_player`).

## v4l2loopback-expbuf (Experimental DMABUF Support)

[`v4l2loopback-expbuf`](https://github.com/daijh/v4l2loopback) is an **experimental** modified version of the original [`v4l2loopback`](https://github.com/v4l2loopback/v4l2loopback) kernel module. This fork adds support for the [`VIDIOC_EXPBUF`](https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/vidioc-expbuf.html#ioctl-vidioc-expbuf)ioctl, allowing V4L2 buffers to be exported as DMABUF file descriptors for efficient zero-copy buffer sharing between user-space applications.

Many thanks to the original `v4l2loopback` authors for their foundational work.

## DMABUF Buffer Flow Sequence

```mermaid
sequenceDiagram
    participant Producer as DMABUF Producer
    participant Module as v4l2loopback-expbuf
    participant Consumer as DMABUF Consumer

    Producer->>Module: VIDIOC_QBUF(V4L2_MEMORY_DMABUF)<br>Queue buffer with dmabuf_fd

    Module->>Module: Extract dmabuf from fd<br>Store dmabuf associated with buffer index

    Consumer->>Module: VIDIOC_DQBUF(V4L2_MEMORY_MMAP)<br>Dequeue buffer index and metadata<br>(**MUST NOT** mmap this buffer)

    Consumer->>Module: VIDIOC_EXPBUF(buffer index)<br>Request dmabuf export

    Module->>Module: Find dmabuf by buffer index<br>Get/Create dmabuf_fd for this dmabuf<br>Increment dmabuf reference count

    Module-->>Consumer: dmabuf_fd

    Consumer->>Consumer: Process data using dmabuf_fd<br>(Display/Encode)

    Consumer->>Module: VIDIOC_QBUF(V4L2_MEMORY_MMAP)<br>Return buffer index to module

    Module->>Module: Decrement queue's dmabuf reference

    Producer->>Module: VIDIOC_DQBUF(V4L2_MEMORY_DMABUF)<br>Acquire original dmabuf

    Module-->>Producer: Return dmabuf for reuse<br>(When dmabuf references = 0)
```

## Demonstration Setup

This demo uses two example applications from the [`v4l2_camera`](https://github.com/daijh/v4l2_camera) repository: `v4l2_clone_device` and `v4l2_player`.

`v4l2_clone_device`: Reads frames from an input V4L2 device (e.g., /dev/video0), acts as the DMABUF producer, copies content into DMABUF buffers, and queues them to the `v4l2loopback-expbuf` device. It can optionally display the frames it reads.

`v4l2_player`: Acts as the DMABUF consumer, dequeues buffers from the `v4l2loopback-expbuf` device (e.g., /dev/video2), exports them as DMABUF file descriptors, and uses these to access and display the video stream.

The data flow is as follows:

```mermaid
graph LR
    InputV4L2[Input V4L2 Device<br/>/dev/video0] --> |Read Buffer| CloneApp["v4l2_clone_device"];
    CloneApp --> |Write DMABUF| OutputV4L2[Output V4L2 Device<br/>v4l2loopback-expbuf<br/>/dev/video2];
    OutputV4L2 --> |Read DMABUF| Player["v4l2_player"];
    CloneApp --> |Display Buffer| SDL2Renderer[SDL2 Window];
    Player --> |Display Buffer| SDL2Renderer2[SDL2 Window];

    style CloneApp fill:#ADD8E6,stroke:#333,stroke-width:2px
    style Player fill:#ADD8E6,stroke:#333,stroke-width:2px
```

## Build `v4l2loopback-expbuf`

1. **Clone the repository:**

    ```shell
    git clone https://github.com/daijh/v4l2loopback.git -b expbuf-dmabuf v4l2loopback-expbuf
    cd v4l2loopback-expbuf
    ```

2. **Build the kernel module:**

    ```shell
    make
    ```

## Build Applications

Instructions for building the `v4l2_clone_device` and `v4l2_player` applications can be found in the `v4l2_camera` repository:

See [`v4l2_camera`](https://github.com/daijh/v4l2_camera) build instructions.

## Run

Follow these steps to run the DMABUF demo:

1. **Load the `v4l2loopback-expbuf` module:**

    ```shell
    cd v4l2loopback-expbuf

    echo "Attempting to remove v4l2loopback module..."
    sudo rmmod -v -f v4l2loopback

    # Set debug=3 to enable verbose driver logging
    echo "Attempting to load v4l2loopback module with parameters..."
    sudo insmod ./v4l2loopback.ko \
        devices=1 \
        exclusive_caps=1 \
        min_buffers=10 \
        max_buffers=16 \
        support_dmabuf=1 \
        card_label="v4l2loopback-expbuf" \
        debug=1

    echo "Verifying v4l2loopback module is loaded..."
    lsmod | grep ^v4l2loopback

    echo "Listing v4l2loopback devices (if created):"
    v4l2-ctl --list-devices | grep "v4l2loopback" -A 2
    ```

2. **Run the DMABUF Producer (`v4l2_clone_device`):**

    ```shell
    # Assume the v4l2loopback-expbuf device is /dev/video2
    ./v4l2_clone_device -i /dev/video0 -o /dev/video2 --width 640 --height 360 --dmabuf
    ```

3. **Run the DMABUF Consumer (`v4l2_player`):**

    ```shell
    ./v4l2_player -i /dev/video2 --width 640 --height 360 --dmabuf
    ```
