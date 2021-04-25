# Add an opacity slider to the Tool Settings

### To be Reviewed By

* borgmanJeremy
* mmahmoudian
* Flameshot developers

<br>

### Authors

* Pr. Sunflower


<br>

### Status: Draft ~~| Discussion | Active | Dropped | Superseded~~

[Pr. Sunflower] This is the first time the authors are redacting a document of this sort and they kindly request to double-check their writings and to assist them in missing parts.


<br>

### Superseded by

N/A

<br>

### Related

* Issue #249
* Issue #1085



<br>

## Problem

Currently the drawing and marking tools in Flameshot only have one, non-customisable opacity setting. This current setting is bad for highlighting text because the Marker tool looks very pale. To compensate the user often has to highlight multiple times which is not convenient and time-consuming.

Here is a comparison of Flameshot's Marker tool with Microsoft's **Snip & Sketch** Highlighter tool:

### Black text on light background

**Flameshot:**
![Flameshot_WHITE](https://user-images.githubusercontent.com/59576952/96623357-8c0a8600-130b-11eb-82e9-05ebbd95a7d9.png)

**Snip & Sketch:**
![Snip_and_Sketch_WHITE](https://user-images.githubusercontent.com/59576952/96623397-9a58a200-130b-11eb-9d27-9a85f4fad504.png)

<br>

### White text on dark background

**Flameshot:**
![Flameshot_BLACK](https://user-images.githubusercontent.com/59576952/96623470-b8260700-130b-11eb-88ff-ff34ea69294c.png)

**Snip & Sketch:**
![Snip_and_Sketch_BLACK](https://user-images.githubusercontent.com/59576952/96623478-bbb98e00-130b-11eb-9e26-59f72cc936a4.png)





<br>

<br>

## Anti-Goals

Flameshot's Marker tool does a very good job at keeping the text readable so this is not linked to this request.

![image](https://user-images.githubusercontent.com/59576952/96624043-806b8f00-130c-11eb-9eb3-ce01d19234df.png)

![image](https://user-images.githubusercontent.com/59576952/96624227-bf99e000-130c-11eb-92e7-d9c6087f110c.png)




<br>

<br>

## Solution

**Add a way to control opacity, like an opacity slider in the Tool Settings.**

Add an opacity slider under "Active thickness".

![Screenshot from 2021-04-25 17-22-06](https://user-images.githubusercontent.com/59576952/115998533-ba3d8b00-a5f8-11eb-8464-da15b42ce9b1.png)





<br>

<br>

## Performance Impact

[Pr. Sunflower:] *I need help for this part.*

Do you anticipate the proposed changes to impact performance in any way? Are there plans to measure and/or mitigate the impact?


<br>

## Backwards Compatibility and Upgrade Path

[Pr. Sunflower:] *I need help for this part.*

Will the regular rolling upgrade process work with these changes?

How do the proposed changes impact backwards-compatibility? Are message or file formats changing?

Is there a need for a deprecation process to provide an upgrade path to users who will need to adjust their applications?

<br>

## Prior Art

[Pr. Sunflower:] *I don't think this part is useful.*

What would be the alternatives to the proposed solution? What would happen if we don’t solve the problem? Why should this proposal be preferred?

<br>

## FAQ

[Pr. Sunflower:] *I need help for this part.*

Answers to questions you’ve commonly been asked after requesting comments for this proposal.

<br>

## Errata

[Pr. Sunflower:] *I need help for this part.*

What are minor adjustments that had to be made to the proposal since it was approved?
