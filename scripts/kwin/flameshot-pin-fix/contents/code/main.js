workspace.windowAdded.connect(function(window) {
    if (!window.caption.startsWith("flameshot-pin")) {
        return;
    }
    window.keepAbove = true;
    window.skipTaskbar = true;
});
