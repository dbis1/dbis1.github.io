-- Aufgabe 2.2
select o_orderkey
from orders o1,
    (select avg(o_totalprice) avgg, o2.o_shippriority, o2.o_orderstatus
      from orders o2,
           (select o_shippriority, o_orderstatus
            from orders
            group by o_shippriority, o_orderstatus) o3
      where
           o2.o_shippriority = o3.o_shippriority or
           o2.o_orderstatus = o3.o_orderstatus
      group by
           o2.o_shippriority, o2.o_orderstatus) o4
where
    o_totalprice < o4.avgg and
    o4.o_shippriority = o1.o_shippriority and
    o4.o_orderstatus = o1.o_orderstatus;

-- Aufgabe 3: Lösung durch Berechnung der Transitiven Hülle
with recursive
  dg (a,b) as  -- gerichteter graph
    (values (1,2), (2,4), (1,3),
            (3,4), (2,5), (5,6), (4,6)),
  ug as        -- ungerichteter graph
    (select a, b from dg
     union all
     select b, a from dg),
 hull (a,b) as (  -- transitive hülle
  select * from ug
  union
  select ull.a, ug.b
  from ug, hull
  where hull.b = ug.a)
select *
from hull
where a = 6
and b = 1;
