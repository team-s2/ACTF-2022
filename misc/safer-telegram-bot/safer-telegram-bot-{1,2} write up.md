## Safer-telegram-bot-{1,2} Write Up

> by jxji

### safer-telegram-bot-1

找 `flag1`：

```js
const user1 = createUser(~~(1 + Math.random() * 1000000), "test", fs.readFileSync(__dirname + "/flag1.txt", "utf8"));
```

找 `user1.flag`：

```js
bot.on("callback_query", async (query) => {
  const callbackData = query.data || "";
  const userId = parseInt(callbackData.split("_")[0]);
  if(userId !== user1.uid) {
    return await sendMessage(chatId, `...not authorized. ...`);
  }
  if(!isAuthorizedUid(query.from.id)) {
    authorizedUids.push({
      // ...
    });
  }
  return await sendMessage(chatId, "...Your flag is `" + toSafeCode(user1.flag) + "`", ...);
}
```

阅读 [Telegram Bot API 文档](https://core.telegram.org/bots/api)，找 `callback_query`：

> #### [CallbackQuery](https://core.telegram.org/bots/api#callbackquery)
>
> This object represents an incoming callback query from a callback button in an [inline keyboard](https://core.telegram.org/bots#inline-keyboards-and-on-the-fly-updating).
>
> | Field | Type                                            | Description                                                  |
> | :---- | :---------------------------------------------- | :----------------------------------------------------------- |
> | id    | String                                          | Unique identifier for this query                             |
> | from  | [User](https://core.telegram.org/bots/api#user) | Sender                                                       |
> | data  | String                                          | *Optional*. Data associated with the callback button. Be aware that the message originated the query can contain no callback buttons with this data. |

也就是说，需要回传的 `data` 的第一个 `_` 前面的部分和 `user1` 的 `uid` 字段的值相同。

找一下这个 callback data 的来源，找到了 `/login` 命令。可以看到，有三种 callback data：

+ `"0_login_callback:" + msg.chat.id + ":" + msg.message_id`
+ `authorizedUids[0].uid + "_login_callback:" + msg.chat.id + ":" + msg.message_id`
+ `"-1_login_callback:" + msg.chat.id + ":" + msg.message_id`

所以只需要在第二个 callback data 出现的时间窗口内点击按钮就可以拿到 flag1。经过验题，在比赛用服务器上这个窗口期的期望大约是 400ms，由于第一个状态会持续 2s 到 16s 不等，所以人手点击可能不太可行。

研究一下 Telegram API，不难搜到主流的两个 Telegram MTProto API Framework：`Telethon`，或 `Pyrogram`。这里给出基于 Pyrogram 的解法：

```python
import asyncio
from pyrogram import Client, filters

api_id = YOUR_API_ID
api_hash = "YOUR_API_HASH"  # 这两个跟着框架教程就能得到

proxy = {
    "scheme": "socks5",  # "socks4", "socks5" and "http" are supported
    "hostname": "127.0.0.1",
    "port": 1080,
}  # 如果有透明代理则无需配置 proxy

bot = Client("my_account", api_id, api_hash, proxy=proxy)

@bot.on_edited_message()
def auth(client, message):
    if message.chat.username == "actfgamebot01bot":
        print("attempting to login as user1...")
        if message.reply_markup and message.reply_markup.inline_keyboard:
            client.request_callback_answer("actfgamebot01bot", message.id, message.reply_markup.inline_keyboard[0][0].callback_data)

bot.run()
```

> 上述代码会在收到 bot 的消息后被触发两次（分别对应两次 `editMessage`），但并不影响获取 flag。

### safer-telegram-bot-2

本题有三种预期解，实际还观察到了一种非预期解法。

root 用户的 userid 固定为 777000，这和 Telegram 官方账户的 userid 相同。换言之，要想办法让 Telegram 官方给 bot 发送 `/iamroot` 消息。通常来讲，这肯定是不现实的；然而，如果在 Google 上搜索 Telegram 777000，可以找到一个 issue：[[BUG] PTB detect anonymous send channel as 777000](https://github.com/python-telegram-bot/python-telegram-bot/issues/2810)。观察截图可以发现，当 Channel 被关联到群组（参见：[Discussion Groups](https://core.telegram.org/api/discussion)）时，会自动将 Channel 中的消息转发到群聊中。这个*转发*的操作实际上通过 user 777000 完成，换言之 bot 会看到该消息的发送者为 userid=777000。

但是，只要尝试将 bot 邀请到群聊，就可以观察到 bot 会自动退出：

````js
bot.on("my_chat_member", async (update) => {
  // this works for both channels and groups... I think so
  if(String(update.chat.id).startsWith("-100")) {
    await sendMessage(update.chat.id, "This bot is not allowed to join groups");
    await bot.leaveChat(update.chat.id);
    return;
  }
});
````

所以核心是要解决 bot 会自动退群的问题。

#### 解法一：我很懂 Telegram

注意到 `my_chat_member` 的回调函数中，检测了 `update.chat.id` 是否为 `-100` 开头。Telegram 的群组和频道使用了差不多的底层代码，它们的 `chatId` 均为 `-100` 开头。然而，了解 Telegram 的人会知道，并不是所有的群组都是 `-100` 开头。这是 Telegram 的历史包袱。Telegram 有两种群聊，一种是 group，另一种是 supergroup。supergroup 和 Channel 用的同一种底层代码，并且相比 group 支持更多的功能，例如设置不同权限的管理员，例如链接到频道并作为 Discussion Group，例如可以拥有 group username 变成公开群，例如可以保存所有历史记录并且拥有更多群成员，例如连续的 message id，等等。Telegram 在极力淡化 group 和 supergroup 的概念。新创建的群聊都是 group，会在用户尝试进行 group 不支持的操作时自动升格为 supergroup。在此过程中，就会发生 chatId 的变化。

因此，不难构造出一种可行的解法：

1. 创建普通群聊（group）
2. 将 bot 邀请到群聊中
3. 将群聊链接到频道，使得群聊成为 Discussion Group（注意：Discussion Group 特指被链接到频道的，以频道评论区身份存在的群聊）
4. 这时，group 自动升格为 supergroup。客户端会提示过往的 100 条历史消息是否对 bot 可见
   + 如果选择不可见，那么一切 OK；
   + 如果选择可见，bot 会重新收到自己的进群 Update，再次触发 `my_chat_member` 回调，从而导致退群。为了避免这种情况，可以先刷出去 100 条消息，再将群聊链接到频道。
5. 在频道中发送 `/iamroot`，获得 flag2。

> 按一个按钮，5 美元；知道按哪个按钮，4995 美元。

#### 解法二：原型链污染

考虑到并不是所有人都很懂 Telegram，因此还设计了原型链污染的解法。

观察 `/addkw key reply` 命令，可以看到，尝试往 `user1` 的 `keywordMap` 的对应 key 里面写入用户指定的 reply：

```js
onText(/^\/addkw (\S+) (\S+)/, async (msg, match) => {
  const keyword = match[1];
  const reply = match[2];
  user1["keywordMap?." + keyword] = () => reply;
  await sendMessage(msg.chat.id, "success");
});
```

看到这里有个 `keywordMap?.`，跟进去看一眼实现：

```js
get(target, prop) {
  const paths = prop.split("?.");
  let current = target;
  for (const path of paths) {
    current = current[path];
    if(!current)
      return undefined;
  }
  return current;
}
```

在 getter 里面按 `?.` 切分了要访问的 key，之后逐层递归进去取值，这样相当于手动实现了一个低配版本的 optional chaining。

然而没有过滤允许访问的 prop 名称，因此可以实现原型链污染。例如，设定 key 为 `__proto__`，那么就访问到了 `keywordMap.__proto__` 也就是 `Object.prototype`。这样，我们就可以轻松覆盖任何（非 primitive type）变量上任意不存在的属性。例如，发送 `/addkw __proto__?.test 1` 给 bot，随后就能污染掉 `Object.prototype.test`：

```js
const a = {};
console.log(a.test);   //  "1"
```

掌握了这一武器，接下来的思路就很多了。这里提供一种可行的解法供参考。

阅读 `node-telegram-bot-api` 的[源代码](https://github.com/yagop/node-telegram-bot-api/blob/d28875154cf89d73e71407e53c7f7738073ca5ff/src/telegram.js#L613)，发现该框架在处理从 Telegram API 拉取到的请求时，采用了逐一判断属性是否存在的方法来确定当前 Update 的类型：

```js
// ...
const pollAnswer = update.poll_answer;
const chatMember = update.chat_member;
const myChatMember = update.my_chat_member;
// ...
} else if (pollAnswer) {
  debug('Process Update poll_answer %j', pollAnswer);
  this.emit('poll_answer', pollAnswer);
} else if (chatMember) {
  debug('Process Update chat_member %j', chatMember);
  this.emit('chat_member', chatMember);
} else if (myChatMember) {
  debug('Process Update my_chat_member %j', myChatMember);
  this.emit('my_chat_member', myChatMember);
// ...
```

故，我们可以污染 `update.my_chat_member` 前面的任何一个取值操作，例如可以污染 `chat_member`，使得 `my_chat_member` 永远不会被触发：

```
/addkw __proto__?.chat_member 1
```

> 注意，为了避免 `__proto__` 被 Telegram 自动渲染为 *proto*，可以使用 Ctrl-Shift-M 将其设置为 monospace 格式。

#### 解法三：条件竞争

这里如果采用 race condition 的思路，需要一些特殊的技巧。这是因为，bot 进群后收到的第一个 Update 一定是 bot 自己被添加到群聊产生的 Update，因此无法使得其它事件回调先于 `my_chat_member` 回调触发。而 Telegram 在将频道中的消息转发到关联群会存在一定的延迟，因此如果先将 bot 拉入群，则基本上必定无法利用成功。故，需要先在 Channel 中发送 `/iamroot`，再邀请 bot 进入关联的讨论群。换言之，条件竞争的窗口期是在 `my_chat_member ` 的 `await sendMessage` 和 `await bot.leaveChat` 期间，使得在频道中发送的 `/iamroot` **恰好被自动转发到群聊**，并且还需要 bot 成功将 flag2 发送到群里。如果在尝试发送 flag2 时 `leaveChat` 已经执行完毕，flag2 就会发送失败，相当于利用没有成功。

总体来讲，由于 Telegram 的特性，`bot.leaveChat` 执行的时间比 `sendMessage` 长一些，使得条件竞争有利用成功的可能，但窗口期较短。

#### 解法四：口球（非预期）

在解法三里可以看到，条件竞争利用成功需要使得 `sendMessage` + `leaveChat` 的时间比 `sendMessage` + `sendMessage` 长。那么，我们可以想办法延长 `sendMessage` + `leaveChat` 所需的时间。于是产生一种思路：尝试先开启全群禁言，再把 bot 拉到群里。

实践表明，这样并没有办法延长 `sendMessage` 所需的时间，但是会产生另外的效果：

```js
bot.on("my_chat_member", async (update) => {
  if(String(update.chat.id).startsWith("-100")) {
    await sendMessage(update.chat.id, "This bot is not allowed to join groups");  //  <--- send failed
    await bot.leaveChat(update.chat.id);
  }
});
```

在第三行，`await sendMessage` 会抛出一个错误（因为 bot 被禁言了），导致第四行的 `leaveChat` 不会被执行。因此 bot 不会退群。由于 `my_chat_member` 事件只会在 bot 被拉入群聊时触发一次，因此该回调函数不会被再次调用。此后，只需要解除全群禁言，并在频道里发送 `/iamroot` 就能得到 flag 了。