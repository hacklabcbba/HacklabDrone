function slinkBench(~)

payload = uint8([1:84] + 48);

messageTX = slink();
messageRX = slink();

% Message TX
count = 0;
tic;
for ii = 1:10000
    messageTX.InitMessage();
    messageTX.Identifier = 27;
    messageTX.LoadPayload(payload);
    messageTX.EndMessage();
    count = count + 1;
end
dt = toc;
fprintf('Time: %0.3f Count: %d\n', dt * 1000, count);

packet = [messageTX.Packet, 0, 0, 0, 0];

% Message RX
count = 0;
tic;
for ii = 1:10000
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
