{% if root %}

<h1>Latest releases</h1><p />

{% else %}

<p><strong>Color scheme:</strong></p>
<p><span style="color: green;">Green: Release</span></p>
<p><span style="color: rgb(255, 196, 0);">Yellow: Beta</span></p>
<p><span style="color: red;">Red: Development</span></p>
<p>White: Unknown version</p>
<p />

{% endif %}

{{ Table|safe }}