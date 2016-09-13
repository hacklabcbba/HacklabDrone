function slinkBench(cycles, payload, packetpad)
if ~exist('cycles'), cycles = 100000; end
if ~exist('payload'), payload = zeros(1, 100); end
if ~exist('pad'), packetpad = [0,0,0,0]; end
payload = uint8(payload);
packetpad = uint8(packetpad);
fprintf('Cycles: %d PayloadSize: %d PadSize: %d\n', cycles, numel(payload), numel(packetpad));

messageTX = slink();
messageRX = slink();

% Message TX
count = 0;
tic;
for ii = 1:cycles
    messageTX.InitMessage();
    messageTX.Identifier = 27;
    messageTX.LoadPayload(payload);
    messageTX.EndMessage();
    count = count + 1;
end
dt = toc;
fprintf('Time: %0.3f Count: %d\n', dt * 1000, count);

packet = [messageTX.Packet, packetpad];

% Message RX
count = 0;
tic;
for ii = 1:cycles
    buffer = packet;
    while numel(buffer)
        [result, buffer] = messageRX.ReceiveMessage(buffer);
        if result == 1
            count = count + 1;
        end
    end
end
dt = toc;
fprintf('Time: %0.3f Count: %d\n', dt * 1000, count);

end
