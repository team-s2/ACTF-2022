## Safer-telegram-bot-{1,2} Write Up

### safer-telegram-bot-1

1. Search for `flag1` in the source code

```js
const user1 = createUser(~~(1 + Math.random() * 1000000), "test", fs.readFileSync(__dirname + "/flag1.txt", "utf8"));
```

2. Search for `user1.flag`

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

3. Read the [Telegram Bot API document](https://core.telegram.org/bots/api)：

> #### [CallbackQuery](https://core.telegram.org/bots/api#callbackquery)
>
> This object represents an incoming callback query from a callback button in an [inline keyboard](https://core.telegram.org/bots#inline-keyboards-and-on-the-fly-updating).
>
> | Field | Type                                            | Description                                                  |
> | :---- | :---------------------------------------------- | :----------------------------------------------------------- |
> | id    | String                                          | Unique identifier for this query                             |
> | from  | [User](https://core.telegram.org/bots/api#user) | Sender                                                       |
> | data  | String                                          | *Optional*. Data associated with the callback button. Be aware that the message originated the query can contain no callback buttons with this data. |

Conclusion: We need to make sure that in the provided `callback_data`, the substring before the first `_` equals to `user1`'s `uid`.

We can tell that the callback data is set in the handler of `/login`, and there are three types of them: 

+ `"0_login_callback:" + msg.chat.id + ":" + msg.message_id`
+ `authorizedUids[0].uid + "_login_callback:" + msg.chat.id + ":" + msg.message_id`
+ `"-1_login_callback:" + msg.chat.id + ":" + msg.message_id`

Hence, we only need to click the button exactly when the second kind of callback data appears. Under the competition environment, the time frame available for this is about 400ms. Since the first type of callback data will last for 2 seconds to 16 seconds, trying to click the button with human hands and expecting the flag to appear is probably not feasible.

After a quick search in Google, we can find two major automated Telegram MTProto API Framework: `Telethon` and `Pyrogram`. Here, a solution based on pyrogram is provided:

```python
import asyncio
from pyrogram import Client, filters

api_id = YOUR_API_ID
api_hash = "YOUR_API_HASH"  # these two values are from https://core.telegram.org/api/obtaining_api_id

bot = Client("my_account", api_id, api_hash)

@bot.on_edited_message()
def auth(client, message):
    if message.chat.username == "actfgamebot01bot":
        print("attempting to login as user1...")
        if message.reply_markup and message.reply_markup.inline_keyboard:
            client.request_callback_answer("actfgamebot01bot", message.id, message.reply_markup.inline_keyboard[0][0].callback_data)

bot.run()
```

> The above code will be triggered twice per a `/login`'s response message, but that's OK.

### safer-telegram-bot-2

There are 3 expected methods to solve this challenge.

#### Background

`root` user's userid is set to 777000, which is the same as Telegram official account's userid. In other words, we need to let the official account send `/iamroot` to the bot. This is not quite possible; however, if we search for "Telegram 777000" on Google, we can find a GitHub issue: [[BUG] PTB detect anonymous send channel as 777000](https://github.com/python-telegram-bot/python-telegram-bot/issues/2810). By observing the screenshot, we can see that when a channel is linked to a group (see also: [Discussion Groups](https://core.telegram.org/api/discussion)), messages sent in the channel will be automatically forwarded to the discussion group. This *forward* operation is actually done by user 777000, which means that bot will think this message comes from Telegram's official account.

But the exploit is not so easy. If we invite the bot to a group, it will quit automatically:

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

Thus, the problem becomes "how to stop the bot from quitting groups".

#### Solution 1: I'm a Telegram Mechanism Expert

We may recognize that in the callback function bind to the `my_chat_member` event, an if statement is used to check whether `update.chat.id` starts with `-100`. Telegram's groups and channels use merely the same underlying codes, and their `chatId`s both start with `-100`. However, people familiar with Telegram will know that not all groups starts with `-100`. This is caused by one of the history burdens of Telegram. Specifically, Telegram has two types of chats: *group* and *supergroup*. Supergroup supports more functions in comparison with group, *e.g.* setting admins with different admin rights, linking to a channel to act as it's discussion group, obtaining a group username so that it becomes a public group, preserving all history messages, etc. The Telegram dev team is devoting much efforts to hide the UX difference between groups and supergroup. Newly created chats are all *groups* by default, which has negative chatId but not starting with -100 (Aha!), and will escalate to supergroup automatically when users try to perform actions that are not supported by groups on it. Note that during the escalation process, the group (which is becoming a supergroup) will discard its old chatId and obtain a new one, which starts with -100.

Knowing this, it is not hard to come up with a viable solution:

1. Create an ordinary *group*
2. Invite the bot into this *group*
3. Link the *group* to your channel, so that the *group* becomes a discussion group, which is necessarily a *supergroup*. 
4. At this point, an automatic escalation will happen on the *group*. The client will prompt the user whether the previous 100 messages is visible to the bot.
   + If you choose false, then everything works fine;
   + Otherwise, the bot will receive those messages (as `Update`s) again, which will probably trigger the `my_chat_member` callback again, resulting in the bot leaving the group (because the now *supergroup* has a chatId starting with `-100`). To avoid this consequence, you can send 100 garbage messages prior to linking the chat to your channel.
5. Send `/iamroot` in your channel, and receive flag2.

#### Solution 2: Prototype Pollution

This path is added for those not familiar with Telegram.

Diving into the handler of `/addkw key reply` command, we can discover that the program tries to write the reply specified by the user into the corresponding entry of `user1`'s `keywordMap`:

```js
onText(/^\/addkw (\S+) (\S+)/, async (msg, match) => {
  const keyword = match[1];
  const reply = match[2];
  user1["keywordMap?." + keyword] = () => reply;
  await sendMessage(msg.chat.id, "success");
});
```

Noticing `keywordMap?.` looks suspicious, let's have a quick glance at its definition:

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

Inside the getter function, the key is split at `?.` , before accessing corresponding values layer-by-layer. By doing so, it implements something similar to the `?.` optional chaining operator. However, here it does not filter the key to be accessed, hence we can construct a prototype pollution. For instance, we set the key to be `__proto__`, and now we can overwrite `Object.prototype`.

Send `/addkw __proto__?.test 1` to bot, and we can pollute `Object.prototype.test`：

```js
const a = {};
console.log(a.test);   //  "1"
```

Read the source code of `node-telegram-bot-api`, and we can know that the framework tries to determine Update type by a series of `if`s:

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

Obviously, we can pollute any attribute access operation before `update.my_chat_member`, *e.g.* `chat_member`, so that the handler of `my_chat_member` will never be invoked:

```
/addkw __proto__?.chat_member 1
```

#### Solution 3: Race Condition

If the method of racing condition is to be carried out, some special techniques might be needed. The very first `Update` the bot will receive after it enters the group is always the Update representing the bot's join chat event, hence making it impossible for other callbacks to be triggered before `my_chat_member`. What's more, the auto-forwarding of channel messages to linked discussion groups in Telegram has a noticeable lag, so if the attacker invite the bot prior to sending the message in channel, the exploitation will never success. 

So, we need to send `/iamroot` in the channel first, and after sleeping for a proper duration, we'll invite the bot to join the discussion group, so that this message is forwarded to the group between the asynchronous `my_chat_member ` handler's `await sendMessage` and `await bot.leaveChat` call.