htmlSessionSpec = `
<div class="container"><button class="btn btn-outline-primary" id="error">Relative error</button></div> </br>
<table id="table-error" class="table table-striped">
    <thead>
        <tr>
            <th>Moment</th>
            <th>Latency intra</th>
            <th>Latency inter</th>
            <th>Bandwidth intra</th>
            <th>Bandwidth inter</th>
            <th>Performance (sec)</th>
            <th>Stable</th>
        </tr>
    </thead>
    <tbody>
    </tbody>
</table>
<div class="container"><button class="btn btn-outline-primary" id="footprint">Footprint</button></div> </br>
<table id="table-footprint" class="table table-striped">
    <thead>
        <tr>
            <th>mean cpu</th>
            <th>mean mem</th>
            <th>mean tx</th>
            <th>mean rx</th>
        </tr>
    </thead>
    <tbody>
    </tbody>
</table>
<table id="table-footprint2" class="table table-striped">
    <thead>
        <tr>
            <th>peak cpu</th>
            <th>min cpu</th>
            <th>peak mem</th>
            <th>min mem</th>
            <th>peak tx</th>
            <th>min tx</th>
            <th>peak rx</th>
            <th>min rx</th>
        </tr>
    </thead>
    <tbody>
    </tbody>
</table>
`

htmlNoData = `
<h4>No Data</h4>
`

htmlSession = `
data...
`

function drawSession(graph) {
    
    d3.selectAll("svg > *").remove();

    var svg = d3.select("svg"),
        width = +svg.attr("width"),
        height = +svg.attr("height");

    var color = d3.scaleOrdinal(d3.schemeCategory20);

    var simulation = d3.forceSimulation()
        .force("link", d3.forceLink().id(function(d) { return d.id; }))
        .force("charge", d3.forceManyBody())
        .force("center", d3.forceCenter(width / 2, height / 2));

    var simulation = d3.forceSimulation()
        .force("link", d3.forceLink().id(function (d) {
            return d.id;
        }))
        .force("charge", d3.forceManyBody())
        .force("center", d3.forceCenter(width / 2, height / 2));

        var link = svg.append("g")
        .attr("class", "links")
      .selectAll("line")
      .data(graph.links)
      .enter().append("line")
        .attr("stroke-width", function(d) { return Math.sqrt(d.value); });
  
    var node = svg.append("g")
        .attr("class", "nodes")
      .selectAll("g")
      .data(graph.nodes)
      .enter().append("g")
      
    var circles = node.append("circle")
        .attr("r", 5)
        .attr("fill", function(d) { return color(d.group); })
        .call(d3.drag()
            .on("start", dragstarted)
            .on("drag", dragged)
            .on("end", dragended));
  
    var lables = node.append("text")
        .text(function(d) {
          return d.name;
        })
        .attr('x', 6)
        .attr('y', 3);
  
    node.append("title")
        .text(function(d) { return d.name; });
  
    simulation
        .nodes(graph.nodes)
        .on("tick", ticked);
  
    simulation.force("link")
        .links(graph.links);
  
    function ticked() {
      link
          .attr("x1", function(d) { return d.source.x; })
          .attr("y1", function(d) { return d.source.y; })
          .attr("x2", function(d) { return d.target.x; })
          .attr("y2", function(d) { return d.target.y; });
  
      node
          .attr("transform", function(d) {
            return "translate(" + d.x + "," + d.y + ")";
          })
    }

    function dragstarted(d) {
        if (!d3.event.active) simulation.alphaTarget(0.01).restart();
        d.fx = d.x;
        d.fy = d.y;
    }

    function dragged(d) {
        d.fx = d3.event.x;
        d.fy = d3.event.y;
    }

    function dragended(d) {
        if (!d3.event.active) simulation.alphaTarget(0);
        d.fx = null;
        d.fy = null;
    }
}

function getSession(id, spec) {
    $("#session").empty();
    var request = $.ajax({
        url: "/testbed/"+id,
        method: "GET",
        dataType: "json"
    });
    
    request.done(function( msg ) {
        $("#session").empty();
        if (spec) {
            $("#session").attr("session",id);
            $("#session").append(htmlSessionSpec);
        }else {
            $("#session").attr("session",id);
            $("#session").append(htmlSessionSpec);
        }
        drawSession(msg["data"]["d3"]);
    });
    
    request.fail(function( jqXHR, textStatus ) {
        $("#session").empty();
        $("#session").append(htmlNoData);
    });
}

function getError(id) {
    var request = $.ajax({
        url: "/testbed/"+id+"/accuracy",
        method: "GET",
        dataType: "json"
    });
    
    request.done(function( msg ) {
        $("#table-error > tbody").empty()
        msg["data"].forEach((data, i) => {
            element = `
            <tr>
                <th>${i}</th>
                <td>${data["L"]["intra"]["mean"].toFixed(2)}</td>
                <td>${data["L"]["inter"]["mean"].toFixed(2)}</td>
                <td>${data["B"]["intra"]["mean"].toFixed(2)}</td>
                <td>${data["B"]["inter"]["mean"].toFixed(2)}</td>
                <td>${data["time"]}</td>
                <td>${data["stable"]}</td>
            </tr>
            `;
            $("#table-error > tbody").append(element)
        });
    });
    
    request.fail(function( jqXHR, textStatus ) {
        alert( "Request failed: " + textStatus );
    });
}

function getFootprint(id) {
    var request = $.ajax({
        url: "/testbed/"+id+"/footprint",
        method: "GET",
        dataType: "json"
    });
    
    request.done(function( msg ) {
        $("#table-footprint > tbody").empty()
        $("#table-footprint2 > tbody").empty()
        msg["data"].forEach((data, i) => {
            element = `
            <tr>
                <td>${data["cpu"]["mean"].toFixed(2)}%</td>
                <td>${data["mem"]["mean"].toFixed(2)}MB</td>
                <td>${(data["tx"]["mean"]/1000).toFixed(2)}KB/s</td>
                <td>${(data["rx"]["mean"]/1000).toFixed(2)}KB/s</td>
            </tr>
            `;
            $("#table-footprint > tbody").append(element)
            element = `
            <tr>
                <td>${data["cpu"]["max"].toFixed(2)}%</td>
                <td>${data["cpu"]["min"].toFixed(2)}%</td>
                <td>${data["mem"]["max"].toFixed(2)}MB</td>
                <td>${data["mem"]["min"].toFixed(2)}MB</td>
                <td>${(data["tx"]["max"]/1000).toFixed(2)}KB/s</td>
                <td>${(data["tx"]["min"]/1000).toFixed(2)}KB/s</td>
                <td>${(data["rx"]["max"]/1000).toFixed(2)}KB/s</td>
                <td>${(data["rx"]["min"]/1000).toFixed(2)}KB/s</td>
            </tr>
            `;
            $("#table-footprint2 > tbody").append(element)
        });
    });
    
    request.fail(function( jqXHR, textStatus ) {
        alert( "Request failed: " + textStatus );
    });
}

$(document).ready(function(){
    $("#sessions").on('click', 'tbody > tr', function() {
        var id = $(this).attr("session");
        var spec = parseInt($(this).attr("spec"));
        getSession(id, spec);
    });

    $("#session").on('click', "#error",function() {
        var id = $("#session").attr("session");
        getError(id);
    });
    $("#session").on('click', "#footprint",function() {
        var id = $("#session").attr("session");
        getFootprint(id);
    });
});
